from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class GiggleConan(ConanFile):
    name = "giggle"
    version = "0.1"

    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "messaging component based on kafka (broker) and avro (serialization)"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*.c*", "include/*.h*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        self.requires("librdkafka/1.9.2")
        self.requires("libavrocpp/1.11.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables = {"CMAKE_INSTALL_INCLUDEDIR" : "include/giggle",
                                     "CMAKE_EXPORT_COMPILE_COMMANDS" : "ON"})
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["giggle"]

