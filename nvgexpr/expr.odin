package nvgexpr

import gl "vendor:OpenGL"
import glfw "vendor:glfw"
import nvg "vendor:nanovg"
import nvg_gl "vendor:nanovg/gl"

premult := true

main :: proc() {
	if !glfw.Init() {
		panic("glfw failed")
	}

	glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, 3)
	glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, 2)
	glfw.WindowHint(glfw.OPENGL_FORWARD_COMPAT, 1)
	glfw.WindowHint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
	glfw.WindowHint(glfw.OPENGL_DEBUG_CONTEXT, 1)

    glfw.WindowHint(glfw.SAMPLES, 4)


	window := glfw.CreateWindow(1000, 600, "NanoVG", nil, nil)

	if window == nil {
		glfw.Terminate()
		panic("glfw window failed")
	}

	glfw.SetKeyCallback(window, key_callback)
	glfw.MakeContextCurrent(window)
	gl.load_up_to(4, 5, glfw.gl_set_proc_address)

    ctx := nvg_gl.Create({.ANTI_ALIAS, .STENCIL_STROKES, .DEBUG})
	defer nvg_gl.Destroy(ctx)

    glfw.SetTime(0)
    prevt := glfw.GetTime()

    for !glfw.WindowShouldClose(window) {
        t := glfw.GetTime()
        dt := t - prevt
        prevt = t

        fw, fh := glfw.GetFramebufferSize(window)
        w, h := glfw.GetWindowSize(window)
        gl.Viewport(0, 0, fw, fh)

        if premult {
            gl.ClearColor(0, 0, 0, 0)
        } else {
            gl.ClearColor(0.3, 0.3, 0.32, 1.0)
        }

        gl.Clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT)
        px_ratio := f32(fw) / f32(fw)

        glfw.SwapBuffers(window)
        glfw.PollEvents()
    }
}

key_callback :: proc "c" (window: glfw.WindowHandle, key, scancode, action, mods: i32) {
    if action != glfw.PRESS {
        return
    }

    switch key {
    case glfw.KEY_ESCAPE:
        {
            glfw.SetWindowShouldClose(window, true)
        }

    case glfw.KEY_P:
        {
            premult = !premult
        }
    }
}