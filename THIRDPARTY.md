# Third party dependencies

Everything the build needs is vendored under `include/` and `lib/`. No package
restore step is required on Windows.

| Library | Version | Location | Linux |
|---|---|---|---|
| SDL3 | 3.2.10 | `include/SDL3/`, `lib/SDL3.lib`, `lib/SDL3.dll` | `libsdl3-dev` |
| GLEW | 2.1.0 | `include/GL/`, `lib/glew32.lib`, `lib/glew32.dll` | `libglew-dev` |
| Dear ImGui | 1.92.7 WIP | `include/imgui/` (built from source) | vendored |
| GLM | 1.x | `include/glm/` (header only) | vendored |
| stb_image | - | `include/stb/` (header only) | vendored |

GLM, stb and ImGui are source-only and compile unchanged on either platform.
Only SDL3 and GLEW ship prebuilt Windows binaries, which is why those two come
from the system package manager on Linux - see the `if(WIN32)` block in the root
`CMakeLists.txt`.

ImGui is compiled from `include/imgui/` only. The core files plus the
`imgui_impl_opengl3` and `imgui_impl_sdl3` backends are built into the
`delusive_imgui` target; the remaining backends in `include/imgui/backend/` are
unused.

## Updating a dependency

1. Replace the headers under `include/<lib>/`.
2. Replace `lib/<lib>.lib` and `lib/<lib>.dll` with the x64 build.
3. Update the version in the table above.
