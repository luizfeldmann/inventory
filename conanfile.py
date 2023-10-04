from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
import os, shutil, json

class Recipe(ConanFile):

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    
    def requirements(self):
        self.requires("grpc/[~1.54]")
        
    def layout(self):
        cmake_layout(self)
        
    def generate(self):
        # Import dependencies 
        deps = CMakeDeps(self)
        deps.generate()

        # Generate toolchain with conan configurations for VS
        tc = CMakeToolchain(self, generator="Ninja Multi-Config")

        # Do not generate user presets due to unsupported schema in VS2019
        tc.user_presets_path = None 

        # Support older versions of the JSON schema
        tc.cache_variables["CMAKE_TOOLCHAIN_FILE"] = os.path.join(self.generators_folder, tc.filename)
        tc.cache_variables["CMAKE_INSTALL_PREFIX"] = "${sourceDir}/out/install"

        # Generate the CMake
        tc.generate()

        # Link the generated presets to the root
        presets_gen = os.path.join(self.generators_folder, "CMakePresets.json")
        presets_usr = os.path.join(self.source_folder, "CMakeUserPresets.json")

        shutil.copyfile(src=presets_gen, dst=presets_usr)
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
