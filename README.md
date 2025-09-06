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

- **Application Layer** - Lifecycle and event management
- **Window System** - Cross-platform window and input handling (GLFW)
- **Renderer** - OpenGL-based graphics pipeline with API abstraction
- **Entity Component System (ECS)** - Flexible game object management
- **Physics** - Integration with Bullet Physics/Jolt Physics
- **Audio** - 3D positional audio system (miniaudio)
- **Asset Pipeline** - Model and texture loading (Assimp)
- **Level Editor** - Scene creation and manipulation tools

## 🛠️ Technology Stack

| Component         | Technology                    |
| ----------------- | ----------------------------- |
| Graphics API      | OpenGL                        |
| Window Management | GLFW                          |
| Physics           | Bullet Physics / Jolt Physics |
| Audio             | miniaudio                     |
| Math Library      | GLM                           |
| Asset Loading     | Assimp                        |
| GUI               | Dear ImGui                    |
| Build System      | CMake                         |
| Logging           | spdlog                        |

## 🚀 Getting Started

### Prerequisites

- **CMake** 3.20 or higher
- **C++20** compatible compiler
- **Git** for version control

### Building on Windows

```bash
# Clone the repository
git clone https://github.com/your-username/Ignis.git
cd Ignis

# Configure and build
cmake --preset x64-debug
cmake --build out/build/x64-debug

# Run the sandbox
./out/build/x64-debug/Sandbox/Sandbox.exe
```

### Building on macOS

```bash
# Install dependencies
brew install cmake ninja

# Clone and build
git clone https://github.com/your-username/Ignis.git
cd Ignis

# Configure and build
cmake --preset arm64-debug
cmake --build out/build/arm64-debug

# Run the sandbox
./out/build/arm64-debug/Sandbox/Sandbox
```

## 📁 Project Structure

```
Ignis/
├── Ignis/                 # Core engine library
│   └── src/
│       ├── Ignis/
│       │   ├── Core/      # Application, events, entry point
│       │   ├── Renderer/  # Graphics and rendering
│       │   ├── Math/      # Vector, matrix operations
│       │   └── Events/    # Event system
│       └── pch.h          # Precompiled headers
├── Sandbox/               # Example application
│   └── src/
├── IgnisEditor/           # Level editor (future)
└── CMakeLists.txt
```

## 🗓️ Development Timeline

| Phase       | Duration     | Focus                           |
| ----------- | ------------ | ------------------------------- |
| **Phase 1** | Sep-Oct 2025 | Foundation & Core Systems       |
| **Phase 2** | Nov-Dec 2025 | Rendering System                |
| **Phase 3** | Jan-Feb 2026 | ECS & Asset Pipeline            |
| **Phase 4** | Mar 2026     | Audio & Tools                   |
| **Phase 5** | Apr 2026     | Game Development & Finalization |

## 👥 Team

- **Chai Ming How** (3036086476) - BEng(CompSc)
- **Liu Sizhe** (3036098041) - BEng(CompSc)

**Supervisor:** Oliveira Bruno  

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE.txt](LICENSE.txt) file for details.

## 🤝 Contributing

This is an academic project developed as part of our Final Year Project. While we appreciate interest, we are not accepting external contributions at this time.

## 📚 Learning Resources

This project is inspired by and learns from:

- [Cherno's Game Engine Series](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)