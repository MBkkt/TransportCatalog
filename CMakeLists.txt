cmake_minimum_required(VERSION 3.14)
project(TransportCatalog)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED) # Команда находит пути к компилятору protoc и библиотеке libprotobuf
include_directories(${Protobuf_INCLUDE_DIRS}) # Добавляем в include path пути к библиотеке protobuf
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # В ${CMAKE_CURRENT_BINARY_DIR} будут файлы, сгенерированные компилятором protoс, путь к ним надо добавить в include path

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
        src/proto/transport_catalog.proto
        src/proto/transport_router.proto
        src/proto/graph.proto
        src/proto/map_renderer.proto
        src/proto/descriptions.proto
        src/proto/svg.proto) # Здесь надо перечислить все ваши .proto-файлы

include_directories("src/public")
add_executable(transport_catalog
        src/main.cpp
        src/private/descriptions.cpp
        src/private/json.cpp
        src/private/requests.cpp
        src/private/sphere.cpp
        src/private/transport_catalog.cpp
        src/private/transport_router.cpp
        src/private/utils.cpp
        src/private/svg.cpp
        src/private/map_renderer.cpp
        src/private/svg_serialize.cpp
        ${PROTO_SRCS}
        ${PROTO_HDRS}) # Здесь надо перечислить все ваши .cpp-файлы, в том числе и сгенерированные protoc'ом
target_link_libraries(transport_catalog ${Protobuf_LIBRARIES}) # компонуем наш исполняемый файл с библиотекой libprotobuf