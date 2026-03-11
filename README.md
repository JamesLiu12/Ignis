# Ignis Game Engine

A C++-based 3D game engine developed as a Final Year Project at The University of Hong Kong, by Chai Ming How and Liu Sizhe.

## 🔥 About

Ignis (Latin for "fire") is a custom game engine built from scratch to understand the fundamental concepts behind modern game development. This project focuses on learning core engine architecture, graphics programming, and real-time systems design.

## 🎯 Project Goals

- Develop a functional 3D game engine capable of producing games
- Gain hands-on experience with graphics programming and engine architecture
- Implement modern C++ design patterns and performance optimization techniques
- Create a foundation for future game development projects

## 🏗️ Architecture

### Core Engine (Ignis)

- **Application Layer** - Generic application lifecycle and event management
- **Window System** - Cross-platform window and input handling (GLFW)
- **Renderer** - OpenGL-based graphics pipeline with API abstraction
- **Entity Component System (ECS)** - Flexible game object management
- **Physics** - Integration with Bullet Physics
- **Audio** - 3D positional audio system (miniaudio)
- **Asset Pipeline** - Model and texture loading (Assimp)
- **Virtual File System** - Asset path resolution and management

### Editor Application

- **Scene Editor** - Visual scene creation and manipulation
- **Property Inspector** - Real-time component editing
- **Debug Panels** - Physics debugging, performance stats, console
- **Asset Browser** - Model and texture management
- **PBR Material Editor** - Real-time material editing with preview

## 🛠️ Technology Stack

| Component         | Technology     |
| ----------------- | -------------- |
| Graphics API      | OpenGL         |
| Window Management | GLFW           |
| Physics           | Bullet Physics |
| Audio             | miniaudio      |
| Math Library      | GLM            |
| Asset Loading     | Assimp         |
| GUI               | Dear ImGui     |
| Build System      | CMake          |
| Logging           | spdlog         |

## 🚀 Getting Started

**You can directly download the release for Windows / MacOS to try our Editor!**

**A sample game project can be found at https://github.com/JamesLiu12/Ignis-Example-Project**

### Prerequisites

- **CMake** 3.20 or higher
- **C++20** compatible compiler
  - Windows: Visual Studio 2022 or later (with C++ desktop development workload)
  - macOS: Xcode Command Line Tools
- **Ninja** build system
  - Windows: Visual Studio 2022 has Ninja by default, or install via Visual Studio installer
  - macOS: `brew install ninja`
- **Git** for version control

### Building on Windows

```bash
# Clone the repository
git clone https://github.com/your-username/Ignis.git
cd Ignis

# Configure with preset
cmake --preset x64-debug
or
cmake --preset x64-release

# Build using preset (builds Editor, Runtime, and script module)
cmake --build --preset x64-debug
or
cmake --build --preset x64-release

# Run the editor
out\build\x64-debug\bin\Editor.exe
or
out\build\x64-release\bin\Editor.exe
```

**Note:** The build process automatically compiles:

- **Editor** - The main editor application
- **Runtime** - Standalone game runtime executable
- **Script Module** - Project-specific game code (DLL)

### Building on macOS

```bash
# Install dependencies
brew install cmake ninja

# Clone and build
git clone https://github.com/your-username/Ignis.git
cd Ignis

# Configure with preset
cmake --preset arm64-debug
or
cmake --preset arm64-release

# Build using preset (builds Editor, Runtime, and script module)
cmake --build --preset arm64-debug
or
cmake --build --preset arm64-release

# Run the editor
out/build/arm64-debug/bin/Editor
or
out/build/arm64-release/bin/Editor
```

**Note:** The build process automatically compiles:

- **Editor** - The main editor application
- **Runtime** - Standalone game runtime executable
- **Script Module** - Project-specific game code (dylib)

## 📁 Project Structure

```
Ignis/
├── Ignis/                    # Core engine library (shared DLL)
│   └── src/
│       ├── Ignis/
│       │   ├── Core/         # Application, events, entry point, logging, file system
│       │   ├── Renderer/     # Graphics pipeline, PBR rendering, shaders
│       │   ├── Scene/        # ECS, entities, components, serialization
│       │   ├── Physics/      # Bullet Physics integration
│       │   ├── Audio/        # 3D audio system (miniaudio)
│       │   ├── Asset/        # VFS, asset loading, importers
│       │   ├── Project/      # Project management, script module loading
│       │   ├── Script/       # Scripting system, native script interface
│       │   ├── UI/           # UI system, text rendering
│       │   ├── Platform/     # Platform-specific implementations (Windows, macOS)
│       │   └── ImGui/        # ImGui integration layer
│       ├── Ignis.h           # Main engine header
│       └── pch.h             # Precompiled headers
├── Editor/                   # Editor application
│   ├── src/
│   │   ├── Editor/
│   │   │   ├── EditorApp.h/cpp         # Main editor application
│   │   │   ├── EditorLayer.h/cpp       # Editor UI, project management, export
│   │   │   ├── EditorSceneLayer.h/cpp  # Scene editing, play mode
│   │   │   ├── PanelManager.h/cpp      # Panel management system
│   │   │   ├── Panels/                 # Editor panels
│   │   │   │   ├── AssetBrowserPanel   # Asset management
│   │   │   │   ├── PropertiesPanel     # Component inspector
│   │   │   │   ├── SceneHierarchyPanel # Entity tree view
│   │   │   │   ├── EditorConsolePanel  # Debug console
│   │   │   │   ├── EngineStatsPanel    # Performance metrics
│   │   │   │   ├── PhysicsDebugPanel   # Physics visualization
│   │   │   │   └── MaterialEditorPanel # PBR material editor
│   │   │   └── Core/                   # Editor utilities
│   │   └── pch.h
│   └── resources/            # Editor resources (shaders, fonts, icons)
├── Runtime/                  # Standalone runtime executable
│   └── src/
│       ├── RuntimeApp.h/cpp         # Runtime application
│       └── RuntimeSceneLayer.h/cpp  # Game scene layer
├── CMakeLists.txt            # Root build configuration
├── CMakePresets.json         # CMake presets for different platforms
└── README.md
```

## 🗓️ Development Timeline

| Phase       | Duration     | Focus                           |
| ----------- | ------------ | ------------------------------- |
| **Phase 1** | Sep-Oct 2025 | Foundation, Core Systems        |
| **Phase 2** | Nov-Dec 2025 | Rendering System                |
| **Phase 3** | Jan-Feb 2026 | ECS, Asset Pipeline             |
| **Phase 4** | Mar 2026     | Audio, tools, runtime           |
| **Phase 5** | Apr 2026     | Game Development & Finalization |

## 👥 Team

- **Chai Ming How** (3036086476) - BEng(CompSc)
- **Liu Sizhe** (3036098041) - BEng(CompSc)

**Supervisor:** Oliveira Bruno

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE.txt](LICENSE.txt) file for details.

Third-party libraries used in this project have their own licenses - see [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for details.

## 🤝 Contributing

This is an academic project developed as part of our Final Year Project. While we appreciate interest, we are not accepting external contributions at this time.

## 📚 Learning Resources

This project is inspired by and learns from:

- [Cherno's Game Engine Series](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)
