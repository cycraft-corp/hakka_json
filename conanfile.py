from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy, get, load
from conan.tools.scm import Version
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
import os
import re

required_conan_version = ">=2.0"


class HakkaJsonConan(ConanFile):
    name = "hakka_json"
    license = "BSL-1.0 OR BSD-3-Clause"
    homepage = "https://github.com/cycraft-corp/hakka_json"
    url = "https://github.com/conan-io/conan-center-index"
    description = "Memory-efficient JSON library with C++23 core and C API - Optimized for minimal runtime footprint"
    topics = ("json", "parser", "cpp23", "memory-efficiency", "embedded", "low-memory")

    package_type = "library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def set_version(self):
        # Extract version from CMakeLists.txt (single source of truth)
        cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
        self.version = re.search(r'VERSION\s+(\d+\.\d+\.\d+)', cmake_file).group(1)

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    def export_sources(self):
        copy(self, "CMakeLists.txt", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "src/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "capi/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "include/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "cmake/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "LICENSE*", src=self.recipe_folder, dst=self.export_sources_folder)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
        # Windows: Force shared=True because ICU doesn't support static linking via MSBuild
        if self.settings.os == "Windows":
            self.options["*"].shared = True

    def validate(self):
        # Windows: Validate that shared=True (ICU limitation)
        if self.settings.os == "Windows" and not self.options.shared:
            raise ConanInvalidConfiguration(
                f"{self.ref} on Windows requires shared=True because ICU does not support "
                "static linking via MSBuild. Use shared=True or build on Linux/macOS for static libraries."
            )
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, "23")

        minimum_versions = {
            "gcc": "11",
            "clang": "15",
            "apple-clang": "14",
            "msvc": "193"
        }

        minimum_version = minimum_versions.get(str(self.settings.compiler))
        if minimum_version and Version(self.settings.compiler.version) < minimum_version:
            raise ConanInvalidConfiguration(
                f"{self.ref} requires C++23, which your {self.settings.compiler} {self.settings.compiler.version} does not support"
            )

    def requirements(self):
        self.requires("nlohmann_json/3.12.0", transitive_headers=True)
        self.requires("tl-expected/1.1.0", transitive_headers=True)

    def source(self):
        # For local/CI builds, sources are exported via export_sources()
        # For Conan Center, fetch from conandata.yml tarball
        if "sources" in self.conan_data and self.version in self.conan_data["sources"]:
            # Only download if URL is not a placeholder
            source_url = self.conan_data["sources"][self.version]["url"]
            if not source_url.startswith("__") and not "0000000000" in self.conan_data["sources"][self.version]["sha256"]:
                get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["HAKKA_JSON_BUILD_TESTS"] = False
        tc.variables["HAKKA_JSON_ENABLE_TBB"] = False
        tc.variables["HAKKA_JSON_USE_SYSTEM_DEPS"] = True
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE*", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

        cmake = CMake(self)
        cmake.install()

        # Copy ICU libraries based on platform
        icu_lib_dir = os.path.join(self.build_folder, "icu-install", "lib")
        icu_bin_dir = os.path.join(self.build_folder, "icu-install", "bin")

        if self.settings.os == "Windows":
            # Windows: Copy ICU DLLs and import libraries
            if os.path.exists(icu_bin_dir):
                copy(self, "icu*.dll", src=icu_bin_dir, dst=os.path.join(self.package_folder, "bin"), keep_path=False)
            if os.path.exists(icu_lib_dir):
                copy(self, "icu*.lib", src=icu_lib_dir, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        else:
            # Unix: Copy ICU static libraries
            if os.path.exists(icu_lib_dir):
                copy(self, "*.a", src=icu_lib_dir, dst=os.path.join(self.package_folder, "lib"), keep_path=False)

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "HakkaJson")
        self.cpp_info.set_property("cmake_target_name", "HakkaJson::core")

        # Library dependencies based on platform and build type
        if self.settings.os == "Windows":
            # Windows: Always shared (DLLs), ICU uses different library names
            self.cpp_info.libs = ["hakka_json_core", "icutu", "icuin", "icuio", "icuuc", "icudt"]
            # Windows shared library needs to find ICU DLLs at runtime
            self.cpp_info.bindirs = ["bin"]
        else:
            # Unix: ICU libraries with standard names
            self.cpp_info.libs = ["hakka_json_core", "icutu", "icui18n", "icuio", "icuuc", "icudata"]

        self.cpp_info.requires = ["nlohmann_json::nlohmann_json", "tl-expected::tl-expected"]

        if self.settings.compiler in ["gcc", "clang", "apple-clang"] and self.settings.arch in ["x86", "armv7", "armv7hf"]:
            self.cpp_info.system_libs.append("atomic")
