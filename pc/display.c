#include "../display.h"
#include "../game_data.h"
#include "../shader_program.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitializeSimpleProgram(SIMPLE_SHADER_PROGRAM* program)
{
	const char vertex_source[] =
		"#if __VERSION__ < 130\n"
		"#define in attribute\n"
		"#define out varying\n"
		"#endif\n"
		"in vec3 in_position;\n"
		"in vec2 in_texture_coord;\n"
		"in vec4 in_color;\n"
		"out vec2 out_texture_coord;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"\tgl_Position = vec4(in_position, 1.0);\n"
		"\tout_texture_coord = in_texture_coord;\n"
		"\tout_color = in_color;\n"
		"}\n";
	const char fragment_source[] =
		"#if __VERSION__ < 130\n"
		"#define in varying\n"
		"#define out_pixel_color gl_FragColor\n"
		"#define texture(samp, uv) texture2D((samp), (uv))\n"
		"#else\n"
		"out out_pixel_color;\n"
		"#endif\n"
		"in vec4 out_color;\n"
		"in vec2 out_texture_coord;\n"
		"out vec4 out_pixel_color;\n"
		"\nuniform sampler2D texture;\n"
		"void main()\n"
		"{\n"
		"\tout_pixel_color = out_color * texture2D(texture, out_texture_coord);\n"
		"}\n";

	(void)memset(program, 0, sizeof(*program));

	(void)InitializeShaderProgram(
		&program->base,
		vertex_source,
		fragment_source,
		"in_position", SHADER_ATTRIBUTE_VERTEX,
		"in_texture_coord", SHADER_ATTRIBUTE_TEXTURE_COORD,
		"in_color", SHADER_ATTRIBUTE_COLOR,
		NULL
	);

	program->texture_uniform = glGetUniformLocation(program->base.program_id, "main_texture");

	return TRUE;
}

void Display(void)
{
	GAME_DATA *game_data = GetGameData();

	SetUpStateOfOpenGL(game_data);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(game_data->programs.draw_square.base.program_id);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, game_data->texture1.id);
	glUniform1i(game_data->programs.draw_square.texture_uniform, 0);

	glBindVertexArray(game_data->vertex_buffer.vertex_array);

	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, NULL);

	glBindVertexArray(0);

	glutSwapBuffers();
}

#ifdef __cplusplus
}
#endif
