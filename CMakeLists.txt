cmake_minimum_required(VERSION 3.14)
project(Navigator)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED) # Команда находит пути к компилятору protoc и библиотеке libprotobuf
include_directories(${Protobuf_INCLUDE_DIRS}) # Добавляем в include path пути к библиотеке protobuf
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # В ${CMAKE_CURRENT_BINARY_DIR} будут файлы, сгенерированные компилятором protoс, путь к ним надо добавить в include path

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/transport_catalog.proto) # Здесь надо перечислить все ваши .proto-файлы
add_executable(main
        main.cpp
        descriptions.cpp
        json.cpp
        requests.cpp
        sphere.cpp
        transport_catalog.cpp
        transport_router.cpp
        utils.cpp
        #svg.cpp
        #map_renderer.cpp
        ${PROTO_SRCS}
        ${PROTO_HDRS}) # Здесь надо перечислить все ваши .cpp-файлы, в том числе и сгенерированные protoc'ом
target_link_libraries(main ${Protobuf_LIBRARIES}) # компонуем наш исполняемый файл с библиотекой libprotobuf