### Java Client

Here is a code sample implementing a GRPC client to EuclidesDB.

| Task                | Command        | Description |
|---------------------|----------------|-------------|
| Compile the project | `gradle build` |             |
| Create a JAR        | `gradle jar`   | `build/libs/euclidesdb-client-0.2.0.jar`|
| Compile the project | `gradle run`   | Runs a sequence of sample queries       |

#### Running the server

`docker run -p 50000:50000 -v ~/database:/database -it euclidesdb/euclidesdb:0.2.0`
