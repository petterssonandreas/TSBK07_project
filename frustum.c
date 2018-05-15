// frustum.c
// Written by Andreas Pettersson and Jonas Ehn as part of the course TSBK07
//
// This file contains the calls that involves the frustum

#include "MicroGlut.h"
#include "VectorUtils3.h"



float const left = -0.2f;
float const right = 0.2f;
float const bottom = -0.1f;
float const top = 0.1f;
float const znear = 0.2f;
float const zfar = 1000.0f;


vec3 VectorReverse(vec3 a)
{
	vec3 zerovec = {0,0,0};
	return VectorSub(zerovec, a);
}


void CreateAndSendProjectionMatrix(GLuint program, GLuint snowprogram, GLuint texprogram)
{
	mat4 projectionMatrix = frustum(left, right, bottom, top, znear, zfar);

	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(snowprogram);
	glUniformMatrix4fv(glGetUniformLocation(snowprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texprogram);
	glUniformMatrix4fv(glGetUniformLocation(texprogram, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
}



void CreateAndSendFrustumCullingParameters(GLuint program, GLuint snowprogram)
{
	float farAngleLR = (float) tan(right/znear);
	float farRight = zfar * (float) atan(farAngleLR);
	float farLeft = - farRight;
	float farAngleTB = (float) tan(top/znear);
	float farTop = zfar * (float) atan(farAngleTB);
	float farBottom = -farTop;

	vec3 ntl = {left, top, -znear};
	vec3 ntr = {right, top, -znear};
	vec3 nbl = {left, bottom, -znear};
	vec3 nbr = {right, bottom, -znear};
	vec3 ftl = {farLeft, farTop, -zfar};
	vec3 ftr = {farRight, farTop, -zfar};
	vec3 fbl = {farLeft, farBottom, -zfar};
	vec3 fbr = {farRight, farBottom, -zfar};

	vec3 farNormal = VectorReverse(CalcNormalVector(ftl, ftr, fbl));
	vec3 leftNormal = CalcNormalVector(ntl, nbl, ftl);
	vec3 rightNormal = VectorReverse(CalcNormalVector(ntr, nbr, fbr));
	vec3 topNormal = VectorReverse(CalcNormalVector(ntl, ntr, ftl));
	vec3 bottomNormal = CalcNormalVector(nbl, nbr, fbl);

	glUseProgram(snowprogram);
	glUniform3fv(glGetUniformLocation(snowprogram, "ntl"), 1, &ntl.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "nbr"), 1, &nbr.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "ftl"), 1, &ftl.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "fbr"), 1, &fbr.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "farNormal"), 1, &farNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "leftNormal"), 1, &leftNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "rightNormal"), 1, &rightNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "topNormal"), 1, &topNormal.x);
	glUniform3fv(glGetUniformLocation(snowprogram, "bottomNormal"), 1, &bottomNormal.x);
	glUseProgram(program);
	glUniform3fv(glGetUniformLocation(program, "ntl"), 1, &ntl.x);
	glUniform3fv(glGetUniformLocation(program, "nbr"), 1, &nbr.x);
	glUniform3fv(glGetUniformLocation(program, "ftl"), 1, &ftl.x);
	glUniform3fv(glGetUniformLocation(program, "fbr"), 1, &fbr.x);
	glUniform3fv(glGetUniformLocation(program, "farNormal"), 1, &farNormal.x);
	glUniform3fv(glGetUniformLocation(program, "leftNormal"), 1, &leftNormal.x);
	glUniform3fv(glGetUniformLocation(program, "rightNormal"), 1, &rightNormal.x);
	glUniform3fv(glGetUniformLocation(program, "topNormal"), 1, &topNormal.x);
	glUniform3fv(glGetUniformLocation(program, "bottomNormal"), 1, &bottomNormal.x);
}