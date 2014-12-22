/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "shader.h"
#include "util.h"

log_category("shader");

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
#define SHADER_STRING(text) STRINGIZE2(text)
#define HASH(x) x

const char* SHADER_PREFIX = SHADER_STRING
                            (
                                HASH(#)version 100 \n
                                HASH(#)ifdef GL_ES \n
                                precision mediump float; \n
                                HASH(#)endif \n
                            );

const char* DEFAULT_VERTEX_SHADER = SHADER_STRING
                                    (
                                        attribute vec2 position;	// position of the vertice
                                        attribute vec4 color;		// color of the vertice
                                        attribute vec2 texCoord;	// texture coordinates
                                        attribute float pointSize;	// size of points

                                        varying vec4 fColor;
                                        varying vec2 fTexCoord;

                                        uniform float cameraDx;
                                        uniform float cameraDy;
                                        uniform float cameraZoom;
                                        uniform mat2 rotationMatrix;
                                        uniform vec2 destinationSize;		// size of the destination texture
                                        mat2 cameraMatrix = rotationMatrix * cameraZoom;

                                        void main()
{
	gl_PointSize = pointSize * cameraZoom;
	vec2 position2d = cameraMatrix * (2. * (position - vec2(cameraDx, cameraDy)) / destinationSize - 1.);
	gl_Position = vec4(position2d, 0.0, 1.0);
	fColor = color;
	fTexCoord = texCoord;
}
                                    );

const char* DEFAULT_FRAGMENT_SHADER_COLOR = SHADER_STRING
        (
            varying vec4 fColor;
            varying vec2 fTexCoord;

            void main()
{
	gl_FragColor = fColor;
}
        );

const char* DEFAULT_FRAGMENT_SHADER_TEX = SHADER_STRING
        (
            uniform sampler2D tex;

            varying vec4 fColor;
            varying vec2 fTexCoord;

            void main()
{
	vec4 color;
	vec4 texval = texture2D(tex, fTexCoord);
	color.rgb = mix(texval.rgb, fColor.rgb, vec3(1.) - fColor.rgb);
	color.a = texval.a * fColor.a;
	gl_FragColor = color;
}
        );

const char* DEFAULT_FRAGMENT_SHADER_TEXPOINT = SHADER_STRING
        (
            uniform sampler2D tex;

            varying vec4 fColor;
            varying vec2 fTexCoord;

            void main()
{
	vec4 color;
	vec2 pos = vec2(gl_PointCoord.x, 1. - gl_PointCoord.y);
	vec4 texval = texture2D(tex, pos);
	color.rgb = mix(texval.rgb, fColor.rgb, vec3(1.) - fColor.rgb);
	color.a = texval.a * fColor.a;
	gl_FragColor = color;
}
        );

Shader *shader_new(GLuint prog_color, GLuint prog_tex, GLuint vert, GLuint frag_color, GLuint frag_tex)
{
	Shader *s = new(Shader, 1);

	s->prog_color = prog_color;
	s->prog_tex = prog_tex;
	s->vert = vert;
	s->frag_color = frag_color;
	s->frag_tex = frag_tex;
	s->ref = 0;

	s->vars[VAR_LOCATION_COLOR].dxLocation = glGetUniformLocation(prog_color, "cameraDx");
	s->vars[VAR_LOCATION_COLOR].dyLocation = glGetUniformLocation(prog_color, "cameraDy");
	s->vars[VAR_LOCATION_COLOR].zoomLocation = glGetUniformLocation(prog_color, "cameraZoom");
	s->vars[VAR_LOCATION_COLOR].rotationMatrixLocation = glGetUniformLocation(prog_color, "rotationMatrix");
	s->vars[VAR_LOCATION_COLOR].destinationSizeLocation = glGetUniformLocation(prog_color, "destinationSize");

	s->vars[VAR_LOCATION_TEX].dxLocation = glGetUniformLocation(prog_tex, "cameraDx");
	s->vars[VAR_LOCATION_TEX].dyLocation = glGetUniformLocation(prog_tex, "cameraDy");
	s->vars[VAR_LOCATION_TEX].zoomLocation = glGetUniformLocation(prog_tex, "cameraZoom");
	s->vars[VAR_LOCATION_TEX].rotationMatrixLocation = glGetUniformLocation(prog_tex, "rotationMatrix");
	s->vars[VAR_LOCATION_TEX].destinationSizeLocation = glGetUniformLocation(prog_tex, "destinationSize");

	return s;
}

void shader_free(Shader *s)
{
	if (!s)
		return;

	glDeleteShader(s->vert);
	glDeleteShader(s->frag_color);
	glDeleteShader(s->frag_tex);
	glDeleteProgram(s->prog_color);
	glDeleteProgram(s->prog_tex);

	free(s);
}

void shader_feed(Shader *s, const char* name, float value)
{
	assert(s);
	assert(name);

	GLint prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

	GLint locColor = glGetUniformLocation(s->prog_color, name);
	GLint locTex = glGetUniformLocation(s->prog_tex, name);

	if (locColor >= 0) {
		glUseProgram(s->prog_color);
		glUniform1f(locColor, value);
	}
	if (locTex >= 0) {
		glUseProgram(s->prog_tex);
		glUniform1f(locTex, value);
	}

	if (locTex < 0 && locColor < 0) {
		log_warning("Cannot feed shader: no location for %s", name);
	}

	glUseProgram(prog);
}

