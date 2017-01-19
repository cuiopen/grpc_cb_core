# gRPC_cb
C++ [gRPC](http://www.grpc.io/) library with callback interface. Depend on grpc but not on grpc++. It is a replacement of grpc++. Easier to use.

(Not ready)

## Build
1. Copy [grpc](https://github.com/grpc/grpc) as third-party/grpc, and build grpc.
	* To ease the building on Windows: [plasticbox/grpc-windows](https://github.com/plasticbox/grpc-windows)
2. Use build/grpc_cb.sln or build/Makefile to build grpc_cb.

## Done
* grpc_cpp_cb_plugin
* grpc_cb library
* helloworld example
* route_guide example

## Todo
1. Delete NewStub()
1. Check thread-safety
1. Rename ...CallBack to On...
1. Make impl sptr const:

		const std::shared_ptr<Impl> impl_sptr_;

1. Run multi stubs in one thread.
1. Lua binding. Convert grpc_byte_buffer to string, which is needed by lua.
1. Timeout
1. Security
1. Support message types other than protobuffer.

## Tutorial
### Generating client and server code
examples/protos/generate.bat is an example to generate client and server interfaces from .proto file, which runs:

		protoc -I . --cpp_out=../cpp_cb/route_guide route_guide.proto
		protoc -I . --grpc_out=../cpp_cb/route_guide --plugin=protoc-gen-grpc=grpc_cpp_cb_plugin.exe route_guide.proto

This generates the following files in directory examples/cpp_cb/route_guide
* `route_guide.pb.h`, generated message classes header
* `route_guide.pb.cc`, the implementation of message classes
* `route_guide.grpc_cb.pb.h`, generated service classes header
* `route_guide.grpc_cb.pb.cc`, the implementation of service classes

The generated namespace ```RouteGuide``` contains
* a ```Stub``` class for clients to call.
* a ```Service``` class for servers to implement.

### Creating the client
See examples/cpp_cb/route_guide/route_guide_cb_client.cc.

#### Creating a stub
1. Create a shared ```Channel```, specifying the server address.
	```cpp
	ChannelSptr channel(new Channel("localhost:50051"));
	```

1. Instantiate a ```Stub```
	```cpp
	Stub stub(channel);
	```

#### Calling service methods
+ Blocking call
	* Simple RPC: ```BlockingGetFeature()```
		```cpp
		Point point = MakePoint(0, 0);
		Feature feature;
		Status status = stub.BlockingGetFeature(point, &feature);
		```

	* Server-side streaming RPC: ```SyncListFeatures()```
		```cpp
		auto sync_reader(stub_->SyncListFeatures(rect));
		while (sync_reader.ReadOne(&feature)) {
			cout << feature.name() << endl;
		}
		Status status = sync_reader.RecvStatus();
		```

	* Client-side streaming RPC: ```SyncRecordRoute()```
		```cpp
		auto sync_writer(stub_->SyncRecordRoute());
		for (int i = 0; i < kPoints; i++) {
			const Feature& f = GetRandomFeature();
			if (!sync_writer.Write(f.location())) {
				// Broken stream.
				break;
			}
		}
		
		// Recv reponse and status.
		RouteSummary stats;
		Status status = sync_writer.Close(&stats);
		```

	* Bidirectional streaming RPC: ```SyncRouteChat()```
		```cpp
		auto sync_reader_writer(stub_->SyncRouteChat());
		auto f = std::async(std::launch::async, [sync_reader_writer]() {
			RunWriteRouteNote(sync_reader_writer);
		});
	
		RouteNote server_note;
		while (sync_reader_writer.ReadOne(&server_note))
			PrintServerNote(server_note);
	
		f.wait();
		Status status = sync_reader_writer.RecvStatus();
		```

		```cpp
		void RunWriteRouteNote(Stus::RouteChat_SyncReaderWriter sync_reader_writer) {
			std::vector<RouteNote> notes{ ... };
			for (const RouteNote& note : notes) {
				sync_reader_writer.Write(note);
				RandomSleep();
			}
			sync_reader_writer.CloseWriting();
		}
		```
+ Asycn call
	* Simple RPC: ```AsyncGetFeature()```
		+ With response callback
			```cpp
			Point point = MakePoint(0, 0);
			stub.AsyncGetFeature(point,
				[](const Feature& feature) {
					PrintFeature(feature);
				});
			```

		+ Ignoring response
			```cpp
			stub.AsyncGetFeature(point);
			```

		+ With error callback
			```cpp
			stub.AsyncGetFeature(point,
				[](const Feature& feature) { PrintFeature(feature); },
				[](const Status& err) {
					cout << err.GetDetails() << endl;
				});  // AsyncGetFeature()
			```

	* Run the stub
		+ Async calls need 
			```cpp
			stub.BlockingRun();  // until stub.Shutdown()
			```

		+ It can run in other thread.
		+ It can be before or after async calls.
		+ ```stub.Shutdown()``` or ```~Stub()``` to end ```stub.BlockingRun()```.

	* Server-side streaming RPC: ```AsyncListFeatures()```
		```cpp
		stub.AsyncListFeatures(rect,
			[](const Feature& feature) {
				cout << feature.name() << endl;
			},
			[&stub](const Status& status) {
				stub.Shutdown();  // To break BlockingRun().
			});
		stub.BlockingRun();  // until stub.Shutdown()
		```

	* Client-side streaming RPC: ```AsyncRecordRoute()```
		```cpp
		auto async_writer = stub.AsyncRecordRoute();
		for (int i = 0; i < kPoints; i++) {
			const Feature& f = GetRandomFeature();
			if (!async_writer.Write(f.location())) {
				break;
			}
		}
		// Recv reponse and status.
		async_writer.Close([](const Status& status, const RouteSummary& resp) {
			if (status.ok())
				cout << resp.point_count() << endl;
		});
		```

	* Bidirectional streaming RPC: ```AsyncRouteChat()```
		```cpp
		std::atomic_bool bReaderDone = false;
		auto async_reader_writer(
			stub.AsyncRouteChat([&bReaderDone](const Status& status) {
				bReaderDone = true;
			}));
		
		async_reader_writer.ReadEach(
			[](const RouteNote& note) { PrintServerNote(note); });

		std::vector<RouteNote> notes{ ... };
		for (const RouteNote& note : notes) {
			async_reader_writer.Write(note);
		}
		async_reader_writer.CloseWriting();
		```

### Creating the server
See examples/cpp_cb/route_guide/route_guide_server.cc.

#### Implementing RouteGuide service

1. Define a ```RouteGuideImpl``` class that implements the generated
   ```RouteGuide::Service``` interface.
   Service is always asynchronous, and there is no AsyncService class.
	```cpp
	class RouteGuideImpl final : public routeguide::RouteGuide::Service {
		...
	}
	```

1. Simple RPC: ```GetFeature()```
	* Reply immediately
		```cpp
		void GetFeature(const Point& point,
				const GetFeature_Replier& replier) override {
			Feature feature;
			feature.set_name("...");
			replier.Reply(feature);
		}
		```

	* Reply later
		```cpp
		void GetFeature(const Point& point,
				const GetFeature_Replier& replier) override {
			GetFeature_Replier replier_copy(replier);
			std::thread thd([replier_copy]() {
				Sleep(1000);
				Feature feature;
				feature.set_name("...");
				replier_copy.Reply(feature);
			});
			thd.detach();
		}
	```

1. Server-side streaming RPC: ```ListFeatures()```
	```cpp
	void ListFeatures(const routeguide::Rectangle& rectangle,
			const ListFeatures_Writer& writer) override {
		ListFeatures_Writer writer_copy(writer)
		std::thread t([writer_copy]() {
			for (const Feature& f : feature_vector) {
				if (!writer_copy.Write(f)) break;
				Sleep(1000);
			}
		});  // thread t
		t.detach();
	}
	```

1. Client-side streaming RPC: ```RecordRoute()```

1. Bidirectional streaming RPC: ```RouteChat()```

#### Starting the server