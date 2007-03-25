// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2000  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT

// tvtxperf.cpp:  Test performance of various ways to specify vertex data

#include "tvtxperf.h"
#include "geomutil.h"
#include "timer.h"
#include "rand.h"
#include "image.h"
#include "codedid.h"
#include "treadpix.h"

namespace {
struct C4UB_N3F_V3F {
	GLubyte c[4];
	GLfloat n[3];
	GLfloat v[3];
};
	
struct C4UB_T2F_V3F {
	GLubyte c[4];
	GLfloat t[2];
	GLfloat v[3];
};

class TvtxBaseTimer: public GLEAN::Timer {
public:
	int nVertices;
	GLuint* indices;
	int nTris;
	GLEAN::Window* w;
	GLEAN::Environment* env;

	TvtxBaseTimer(int v, GLuint* i, int t, GLEAN::Window* win,
		      GLEAN::Environment* e) {
		nVertices = v;
		indices   = i;
		nTris     = t;
		w         = win;
		env       = e;
	}

	virtual double compute(double t) { return nTris/t; }
	virtual void premeasure() {
		// Clear both front and back buffers and swap, to avoid
		// confusing this test with results of the previous
		// test:
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		w->swap();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	virtual void postmeasure() { w->swap(); }
	virtual void preop() { env->quiesce(); glFinish(); }
	virtual void postop() { glFinish(); }
};

class ColoredLit_imIndTri: public TvtxBaseTimer {
public:
	C4UB_N3F_V3F* data;
	ColoredLit_imIndTri(int v, C4UB_N3F_V3F* c, int t, GLEAN::Window* w,
			    GLEAN::Environment* env):
		TvtxBaseTimer(v, 0, t, w, env) {
		data = c;
	}
	
	virtual void op() {
		C4UB_N3F_V3F* p = data;
		glBegin(GL_TRIANGLES);
		// Assume that the data is complete, thus allowing us
		// to unroll 3X and do one tri per iteration rather than
		// one vertex.
		for (int i = nVertices / 3; i; --i) {
			glColor4ubv(p[0].c);
			glNormal3fv(p[0].n);
			glVertex3fv(p[0].v);
			glColor4ubv(p[1].c);
			glNormal3fv(p[1].n);
			glVertex3fv(p[1].v);
			glColor4ubv(p[2].c);
			glNormal3fv(p[2].n);
			glVertex3fv(p[2].v);
			p += 3;
		}
		glEnd();
	}
}; // coloredLit_imIndTri

class ColoredTex_imIndTri: public TvtxBaseTimer {
public:
	C4UB_T2F_V3F* data;
	ColoredTex_imIndTri(int v, C4UB_T2F_V3F* c, int t, GLEAN::Window* w,
			    GLEAN::Environment* env):
		TvtxBaseTimer(v, 0, t, w, env) {
		data = c;
	}

	virtual void op() {
		C4UB_T2F_V3F* p = data;
		glBegin(GL_TRIANGLES);
		// Assume that the data is complete, thus allowing us
		// to unroll 3X and do one tri per iteration rather than
		// one vertex.
		for (int i = nVertices / 3; i; --i) {
			glColor4ubv(p[0].c);
			glTexCoord2fv(p[0].t);
			glVertex3fv(p[0].v);
			glColor4ubv(p[1].c);
			glTexCoord2fv(p[0].t);
			glVertex3fv(p[1].v);
			glColor4ubv(p[2].c);
			glTexCoord2fv(p[0].t);
			glVertex3fv(p[2].v);
			p += 3;
		}
		glEnd();
	}
}; // coloredTex_imIndTri

class ColoredLit_imTriStrip: public TvtxBaseTimer {
public:
	C4UB_N3F_V3F* data;
	ColoredLit_imTriStrip(int v, C4UB_N3F_V3F* c, int t,
			      GLEAN::Window* w, GLEAN::Environment* env):
		TvtxBaseTimer(v, 0, t, w, env) {
		data = c;
	}

	virtual void op() {
		C4UB_N3F_V3F* p = data;
		glBegin(GL_TRIANGLE_STRIP);

		int n = (nVertices + 3) >> 2;
		// Duff's device.  Yes, this is legal C (and C++).
		// See Stroustrup, 3rd ed., p. 141
		switch (nVertices & 0x3) {
		case 0:	do {
			glColor4ubv(p->c);
			glNormal3fv(p->n);
			glVertex3fv(p->v);
			++p;
		case 3:
			glColor4ubv(p->c);
			glNormal3fv(p->n);
			glVertex3fv(p->v);
			++p;
		case 2:
			glColor4ubv(p->c);
			glNormal3fv(p->n);
			glVertex3fv(p->v);
			++p;
		case 1:
			glColor4ubv(p->c);
			glNormal3fv(p->n);
			glVertex3fv(p->v);
			++p;
		} while (--n > 0);
		}
		glEnd();
	}
}; // coloredLit_imTriStrip

class ColoredTex_imTriStrip: public TvtxBaseTimer {
public:
	C4UB_T2F_V3F* data;
	
	ColoredTex_imTriStrip(int v, C4UB_T2F_V3F* c, int t,
			      GLEAN::Window* w, GLEAN::Environment* env):
		TvtxBaseTimer(v, 0, t, w, env) {
		data = c;
	}
	
	virtual void op() {
		C4UB_T2F_V3F* p = data;
		glBegin(GL_TRIANGLE_STRIP);
		
		int n = (nVertices + 3) >> 2;
		// Duff's device.  Yes, this is legal C (and C++).
		// See Stroustrup, 3rd ed., p. 141
		switch (nVertices & 0x3) {
		case 0:	do {
			glColor4ubv(p->c);
			glTexCoord2fv(p->t);
			glVertex3fv(p->v);
			++p;
		case 3:
			glColor4ubv(p->c);
			glTexCoord2fv(p->t);
			glVertex3fv(p->v);
			++p;
		case 2:
			glColor4ubv(p->c);
			glTexCoord2fv(p->t);
			glVertex3fv(p->v);
			++p;
		case 1:
			glColor4ubv(p->c);
			glTexCoord2fv(p->t);
			glVertex3fv(p->v);
			++p;
		} while (--n > 0);
		}
		glEnd();
	}
}; // coloredTex_imTriStrip

class daIndTriTimer: public TvtxBaseTimer {
public:
	daIndTriTimer(int v, GLuint* i, int t, GLEAN::Window* w,
		      GLEAN::Environment* env):
		TvtxBaseTimer(v, i, t, w, env) {
	}
	virtual void op() {glDrawArrays(GL_TRIANGLES, 0, nVertices); }
}; // daIndTriTimer

class daTriStripTimer: public TvtxBaseTimer {
public:
	daTriStripTimer(int v, int t, GLEAN::Window* w,
			GLEAN::Environment* env):
		TvtxBaseTimer(v, 0, t, w, env) {
	}
	virtual void op() { glDrawArrays(GL_TRIANGLE_STRIP, 0, nVertices); }
}; // daTriStripTimer

class deIndTriTimer: public TvtxBaseTimer {
public:
	deIndTriTimer(int v, GLuint* i, int t, GLEAN::Window* w,
		      GLEAN::Environment* env):
		TvtxBaseTimer(v, i, t, w, env) {
	}
	virtual void op() {
		glDrawElements(GL_TRIANGLES, nVertices, GL_UNSIGNED_INT,
			       indices);
	}
}; // deIndTriTimer

class deTriStripTimer: public TvtxBaseTimer {
public:
	deTriStripTimer(int v, GLuint* i, int t, GLEAN::Window* w,
			GLEAN::Environment* env):
		TvtxBaseTimer(v, i, t, w, env) {
	}
	virtual void op() {
		glDrawElements(GL_TRIANGLE_STRIP, nVertices, GL_UNSIGNED_INT,
			       indices);
	}
}; // deTriStripTimer


class callDListTimer: public TvtxBaseTimer {
public:
	int dList;
	callDListTimer(int d, int t, GLEAN::Window* w,
		       GLEAN::Environment* env):
		TvtxBaseTimer(0, 0, t, w, env) {
		dList    = d;
	}
	virtual void op() { glCallList(dList); }
}; // callDList

void
logStats1(const char* title, GLEAN::VPSubResult& r,
    GLEAN::Environment* env) {
	env->log << '\t' << title << " rate = "
		<< r.tps << " tri/sec.\n"
		<< "\t\tRange of valid measurements = ["
		<< r.tpsLow << ", " << r.tpsHigh << "]\n"
		<< "\t\tImage sanity check "
		<< (r.imageOK? "passed\n": "failed\n")
		<< "\t\tImage consistency check "
		<< (r.imageMatch? "passed\n": "failed\n");

} // logStats1

void
diffHeader(bool& same, const string& name,
    GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
	if (same) {
		same = false;
		env->log << name << ":  DIFF "
			<< config->conciseDescription() << '\n';
	}
} // diffHeader

void
failHeader(bool& pass, const string& name,
    GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
	if (pass) {
		pass = false;
		env->log << name << ":  FAIL "
			<< config->conciseDescription() << '\n';
	}
} // failHeader

void
doComparison(const GLEAN::VPSubResult& oldR,
    const GLEAN::VPSubResult& newR,
    GLEAN::DrawingSurfaceConfig* config,
    bool& same, const string& name, GLEAN::Environment* env,
    const char* title) {
	if (newR.tps < oldR.tpsLow) {
		int percent = static_cast<int>(
			100.0 * (oldR.tps - newR.tps) / newR.tps + 0.5);
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name
			<< " may be " << percent << "% faster on "
			<< title << " drawing.\n";
	}
	if (newR.tps > oldR.tpsHigh) {
		int percent = static_cast<int>(
			100.0 * (newR.tps - oldR.tps) / oldR.tps + 0.5);
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db2Name
			<< " may be " << percent << "% faster on "
			<< title << " drawing.\n";
	}
	if (newR.imageOK != oldR.imageOK) {
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name << " image check "
			<< (oldR.imageOK? "passed\n": "failed\n");
		env->log << '\t' << env->options.db2Name << " image check "
			<< (newR.imageOK? "passed\n": "failed\n");
	}
	if (newR.imageMatch != oldR.imageMatch) {
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name << " image compare "
			<< (oldR.imageMatch? "passed\n": "failed\n");
		env->log << '\t' << env->options.db2Name << " image compare "
			<< (newR.imageMatch? "passed\n": "failed\n");
	}
} // doComparison

bool
imagesDiffer(GLEAN::Image& testImage, GLEAN::Image& goldenImage) {
	GLEAN::Image::Registration imageReg(testImage.reg(goldenImage));
	return (imageReg.stats[0].max()
		+ imageReg.stats[1].max()
		+ imageReg.stats[2].max()) != 0.0;
} // imagesDiffer

void
missingSome(GLEAN::Environment* env, const char* title) {
	env->log << '\t' << title << " rendering is missing\n"
			<< "\t\tsome triangles.\n";
} // missingSome

void
theyDiffer(GLEAN::Environment* env, const char* title) {
	env->log << '\t' << title << " image differs from\n"
		<< "\t\tthe reference image.\n";
} // theyDiffer

void
verifyVtxPerf(GLEAN::Image& testImage, GLEAN::RGBCodedID& colorGen,
    int firstID, int lastID, GLEAN::Image& refImage,
    bool& passed, string& name, GLEAN::DrawingSurfaceConfig* config,
    GLEAN::VPSubResult& res, GLEAN::Environment* env, const char* title) {

	// Verify that the entire range of RGB coded identifiers is
	// present in the image.  (This is an indicator that all triangles
	// were actually drawn.)
	testImage.read(0, 0);
	if (!colorGen.allPresent(testImage, firstID, lastID)) {
		failHeader(passed, name, config, env);
		missingSome(env, title);
		res.imageOK = false;
	}

	// Verify that the test image is the same as the reference image.
	if (imagesDiffer(testImage, refImage)) {
		failHeader(passed, name, config, env);
		theyDiffer(env, title);
		res.imageMatch = false;
	}
} // verify

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
ColoredLitPerf::runOne(VPResult& r, Window& w) {
	// Don't bother running if the ExactRGBA test for this display
	// surface configuration failed:
	vector<ExactRGBAResult*>::const_iterator erRes;
	for (erRes = exactRGBATest.results.begin();
	    erRes != exactRGBATest.results.end();
	    ++erRes)
		if ((*erRes)->config == r.config)
			break;
	if (erRes == exactRGBATest.results.end() || !(*erRes)->ub.pass) {
		r.skipped = true;
		r.pass = false;
		return;
	}

	bool passed = true;
	PFNGLLOCKARRAYSEXTPROC glLockArraysEXT = 0;
	PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT = 0;
	if (GLUtils::haveExtension("GL_EXT_compiled_vertex_array")) {
		glLockArraysEXT = reinterpret_cast<PFNGLLOCKARRAYSEXTPROC>
			(GLUtils::getProcAddress("glLockArraysEXT"));
		glUnlockArraysEXT = reinterpret_cast<PFNGLUNLOCKARRAYSEXTPROC>
			(GLUtils::getProcAddress("glUnlockArraysEXT"));
	}

	Image imTriImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);
	Image testImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);

	// Make colors deterministic, so we can check them:
	RGBCodedID colorGen(r.config->r, r.config->g, r.config->b);
	int IDModulus = colorGen.maxID() + 1;

	// We need to minimize the number of pixels per triangle, so that
	// we're measuring vertex-processing rate rather than fill rate.
	// However, we'd also like to guarantee that every triangle covers
	// at least one pixel, so that we can confirm drawing actually took
	// place.  As a compromise, we'll choose a number of triangles that
	// yields approximately 3 pixels per triangle.
	// We're drawing a filled spiral that approximates a circular area,
	// so pi * (drawingSize/2)**2 / nTris = 3 implies...
	const int nTris = static_cast<int>
		(((3.14159 / 4.0) * drawingSize * drawingSize) / 3.0 + 0.5);
	int nVertices = nTris * 3;
	int lastID = min(IDModulus - 1, nTris - 1);

	C4UB_N3F_V3F *c4ub_n3f_v3f = new C4UB_N3F_V3F[nVertices];
	SpiralTri2D it(nTris, 0, drawingSize, 0, drawingSize);
	int k = 0;
	for (int j = 0; j < nTris; ++j) {
		float* t = it(j);
		GLubyte r, g, b;
		colorGen.toRGB(j % IDModulus, r, g, b);

		c4ub_n3f_v3f[k+0].c[0] = r;
		c4ub_n3f_v3f[k+0].c[1] = g;
		c4ub_n3f_v3f[k+0].c[2] = b;
		c4ub_n3f_v3f[k+0].c[3] = 0xFF;
		c4ub_n3f_v3f[k+0].n[0] = 0.0;
		c4ub_n3f_v3f[k+0].n[1] = 0.0;
		c4ub_n3f_v3f[k+0].n[2] = 1.0;
		c4ub_n3f_v3f[k+0].v[0] = t[0];
		c4ub_n3f_v3f[k+0].v[1] = t[1];
		c4ub_n3f_v3f[k+0].v[2] = 0.0;

		c4ub_n3f_v3f[k+1].c[0] = r;
		c4ub_n3f_v3f[k+1].c[1] = g;
		c4ub_n3f_v3f[k+1].c[2] = b;
		c4ub_n3f_v3f[k+1].c[3] = 0xFF;
		c4ub_n3f_v3f[k+1].n[0] = 0.0;
		c4ub_n3f_v3f[k+1].n[1] = 0.0;
		c4ub_n3f_v3f[k+1].n[2] = 1.0;
		c4ub_n3f_v3f[k+1].v[0] = t[2];
		c4ub_n3f_v3f[k+1].v[1] = t[3];
		c4ub_n3f_v3f[k+1].v[2] = 0.0;

		c4ub_n3f_v3f[k+2].c[0] = r;
		c4ub_n3f_v3f[k+2].c[1] = g;
		c4ub_n3f_v3f[k+2].c[2] = b;
		c4ub_n3f_v3f[k+2].c[3] = 0xFF;
		c4ub_n3f_v3f[k+2].n[0] = 0.0;
		c4ub_n3f_v3f[k+2].n[1] = 0.0;
		c4ub_n3f_v3f[k+2].n[2] = 1.0;
		c4ub_n3f_v3f[k+2].v[0] = t[4];
		c4ub_n3f_v3f[k+2].v[1] = t[5];
		c4ub_n3f_v3f[k+2].v[2] = 0.0;

		k += 3;
	}

	GLuint *indices = new GLuint[nVertices];
	for (k = 0; k < nVertices; ++k)
		indices[k] = k;

	GLUtils::useScreenCoords(drawingSize, drawingSize);

	// Diffuse white light at infinity, behind the eye:
	GLUtils::Light light(0);
	light.ambient(0, 0, 0, 0);
	light.diffuse(1, 1, 1, 0);
	light.specular(0, 0, 0, 0);
	light.position(0, 0, 1, 0);
	light.spotCutoff(180);
	light.constantAttenuation(1);
	light.linearAttenuation(0);
	light.quadraticAttenuation(0);
	light.enable();

	GLUtils::LightModel lm;
	lm.ambient(0, 0, 0, 0);
	lm.localViewer(false);
	lm.twoSide(false);
	lm.colorControl(GL_SINGLE_COLOR);

	glFrontFace(GL_CCW);
	glEnable(GL_NORMALIZE);
	GLUtils::Material mat;
	mat.ambient(0, 0, 0, 1);
	mat.ambientAndDiffuse(1, 1, 1, 1);
	mat.specular(0, 0, 0, 1);
	mat.emission(0, 0, 0, 1);
	mat.shininess(0);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glReadBuffer(GL_FRONT);

	////////////////////////////////////////////////////////////
	// Immediate-mode independent triangles
	////////////////////////////////////////////////////////////
	ColoredLit_imIndTri coloredLit_imIndTri(nVertices, c4ub_n3f_v3f,
						nTris, &w, env);
	coloredLit_imIndTri.measure(5, &r.imTri.tpsLow, &r.imTri.tps,
				    &r.imTri.tpsHigh);
	imTriImage.read(0, 0);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.imTri, env,
	       "Immediate-mode independent triangle");

	////////////////////////////////////////////////////////////
	// Display-listed independent triangles
	////////////////////////////////////////////////////////////
	int dList = glGenLists(1);
	glNewList(dList, GL_COMPILE);
	coloredLit_imIndTri.op();
	glEndList();
	callDListTimer callDList(dList, nTris, &w, env);
	callDList.measure(5, &r.dlTri.tpsLow, &r.dlTri.tps, &r.dlTri.tpsHigh);
	glDeleteLists(dList, 1);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.dlTri, env,
	       "Display-listed independent triangle");

	////////////////////////////////////////////////////////////
	// DrawArrays on independent triangles
	////////////////////////////////////////////////////////////
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].c);
	glEnableClientState(GL_COLOR_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].n);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].v);
	glEnableClientState(GL_VERTEX_ARRAY);

	daIndTriTimer daIndTri(nVertices, indices, nTris, &w, env);
	daIndTri.measure(5, &r.daTri.tpsLow, &r.daTri.tps, &r.daTri.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.daTri, env,
		"DrawArrays independent triangle");

	////////////////////////////////////////////////////////////
	// Locked DrawArrays on independent triangles
	//	XXX This is probably unrealistically favorable to
	//	locked arrays.
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	daIndTri.measure(5, &r.ldaTri.tpsLow, &r.ldaTri.tps,
			 &r.ldaTri.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldaTri.tps = r.ldaTri.tpsLow = r.ldaTri.tpsHigh = 0.0;
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.ldaTri, env,
		"Locked DrawArrays independent triangle");

	////////////////////////////////////////////////////////////
	// DrawElements on independent triangles
	////////////////////////////////////////////////////////////
	deIndTriTimer deIndTri(nVertices, indices, nTris, &w, env);
	deIndTri.measure(5, &r.deTri.tpsLow, &r.deTri.tps, &r.deTri.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.deTri, env,
	       "DrawElements independent triangle");

	////////////////////////////////////////////////////////////
	// Locked DrawElements on independent triangles
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	deIndTri.measure(5, &r.ldeTri.tpsLow, &r.ldeTri.tps,
			 &r.ldeTri.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldeTri.tps = r.ldeTri.tpsLow = r.ldeTri.tpsHigh = 0.0;
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.ldeTri, env,
	       "Locked DrawElements independent triangle");

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	delete[] c4ub_n3f_v3f;
	delete[] indices;

	// Now we test triangle strips, rather than independent triangles.

	nVertices = nTris + 2;
	lastID = min(IDModulus - 1, nTris - 1);

	c4ub_n3f_v3f = new C4UB_N3F_V3F[nVertices];
	SpiralStrip2D is(nVertices, 0, drawingSize, 0, drawingSize);
	for (int j2 = 0; j2 < nVertices; ++j2) {
		float* t = is(j2);
		GLubyte r, g, b;
		// Take care to get the correct color on the provoking vertex:
		colorGen.toRGB((j2 - 2) % IDModulus, r, g, b);

		c4ub_n3f_v3f[j2].c[0] = r;
		c4ub_n3f_v3f[j2].c[1] = g;
		c4ub_n3f_v3f[j2].c[2] = b;
		c4ub_n3f_v3f[j2].c[3] = 0xFF;
		c4ub_n3f_v3f[j2].n[0] = 0.0;
		c4ub_n3f_v3f[j2].n[1] = 0.0;
		c4ub_n3f_v3f[j2].n[2] = 1.0;
		c4ub_n3f_v3f[j2].v[0] = t[0];
		c4ub_n3f_v3f[j2].v[1] = t[1];
		c4ub_n3f_v3f[j2].v[2] = 0.0;
	}

	indices = new GLuint[nVertices];
	for (int j3 = 0; j3 < nVertices; ++j3)
		indices[j3] = j3;

	////////////////////////////////////////////////////////////
	// Immediate-mode triangle strips
	////////////////////////////////////////////////////////////
	ColoredLit_imTriStrip coloredLit_imTriStrip(nVertices, c4ub_n3f_v3f,
						    nTris, &w, env);
	coloredLit_imTriStrip.measure(5, &r.imTS.tpsLow, &r.imTS.tps,
				      &r.imTS.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.imTS, env,
	       "Immediate-mode triangle strip");

	////////////////////////////////////////////////////////////
	// Display-listed triangle strips
	////////////////////////////////////////////////////////////
	dList = glGenLists(1);
	glNewList(dList, GL_COMPILE);
	coloredLit_imTriStrip.op();
	glEndList();
	callDList.dList = dList;
	callDList.measure(5, &r.dlTS.tpsLow, &r.dlTS.tps, &r.dlTS.tpsHigh);
	glDeleteLists(dList, 1);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.dlTS, env,
	       "Display-listed triangle strip");

	////////////////////////////////////////////////////////////
	// DrawArrays on triangle strips
	////////////////////////////////////////////////////////////
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].c);
	glEnableClientState(GL_COLOR_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].n);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].v);
	glEnableClientState(GL_VERTEX_ARRAY);

	daTriStripTimer daTriStrip(nVertices, nTris, &w, env);
	daTriStrip.measure(5, &r.daTS.tpsLow, &r.daTS.tps, &r.daTS.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.daTS, env,
	       "DrawArrays triangle strip");

	////////////////////////////////////////////////////////////
	// Locked DrawArrays on triangle strips
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	daTriStrip.measure(5, &r.ldaTS.tpsLow, &r.ldaTS.tps, &r.ldaTS.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldaTS.tps = r.ldaTS.tpsLow = r.ldaTS.tpsHigh = 0.0;
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.ldaTS, env,
	       "Locked DrawArrays triangle strip");

	////////////////////////////////////////////////////////////
	// DrawElements on triangle strips
	////////////////////////////////////////////////////////////
	deTriStripTimer deTriStrip(nVertices, indices, nTris, &w, env);
	deTriStrip.measure(5, &r.deTS.tpsLow, &r.deTS.tps, &r.deTS.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.deTS, env,
	       "DrawElements triangle strip");

	////////////////////////////////////////////////////////////
	// Locked DrawElements on triangle strips
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	deTriStrip.measure(5, &r.ldeTS.tpsLow, &r.ldeTS.tps, &r.ldeTS.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldeTS.tps = r.ldeTS.tpsLow = r.ldeTS.tpsHigh = 0.0;
	
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.ldeTS, env,
		"Locked DrawElements triangle strip");

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	delete[] c4ub_n3f_v3f;
	delete[] indices;
	
	r.pass = passed;
	r.skipped = false;
} // ColoredLitPerf::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
ColoredLitPerf::logOne(VPResult& r) {
	if (r.skipped) {
		env->log << name << ":  NOTE ";
		logConcise(r);
		env->log << "\tTest skipped; prerequisite test "
			 << exactRGBATest.name
			 << " failed or was not run\n";
		return;
	}
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else env->log << '\n'; // because verify logs failure
	logStats(r, env);
} // ColoredLitPerf::logOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
ColoredLitPerf::compareOne(VPResult& oldR, VPResult& newR) {
	if (oldR.skipped || newR.skipped) {
		env->log << name
			 << ((oldR.skipped && newR.skipped)? ":  SAME "
			 	: ":  DIFF ")
			 << newR.config->conciseDescription()
			 << '\n';
		if (oldR.skipped)
			 env->log << "\t"
				  << env->options.db1Name
				  << " skipped\n";
		if (newR.skipped)
			 env->log << "\t"
				  << env->options.db2Name
				  << " skipped\n";
		env->log << "\tNo comparison is possible.\n";
		return;
	}

	bool same = true;
	doComparison(oldR.imTri, newR.imTri, newR.config, same, name,
		env, "immediate-mode independent triangle");
	doComparison(oldR.dlTri, newR.dlTri, newR.config, same, name,
		env, "display-listed independent triangle");
	doComparison(oldR.daTri, newR.daTri, newR.config, same, name,
		env, "DrawArrays independent triangle");
	doComparison(oldR.ldaTri, newR.ldaTri, newR.config, same, name,
		env, "Locked DrawArrays independent triangle");
	doComparison(oldR.deTri, newR.deTri, newR.config, same, name,
		env, "DrawElements independent triangle");
	doComparison(oldR.ldeTri, newR.ldeTri, newR.config, same, name,
		env, "Locked DrawElements independent triangle");
	doComparison(oldR.imTS, newR.imTS, newR.config, same, name,
		env, "immediate-mode triangle strip");
	doComparison(oldR.dlTS, newR.dlTS, newR.config, same, name,
		env, "display-listed triangle strip");
	doComparison(oldR.daTS, newR.daTS, newR.config, same, name,
		env, "DrawArrays triangle strip");
	doComparison(oldR.ldaTS, newR.ldaTS, newR.config, same, name,
		env, "Locked DrawArrays triangle strip");
	doComparison(oldR.deTS, newR.deTS, newR.config, same, name,
		env, "DrawElements triangle strip");
	doComparison(oldR.ldeTS, newR.ldeTS, newR.config, same, name,
		env, "Locked DrawElements triangle strip");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n\t"
			<< env->options.db2Name
			<< " test time falls within the "
			<< "valid measurement range of\n\t"
			<< env->options.db1Name
			<< " test time; both have the same"
			<< " image comparison results.\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR, env);
		env->log << env->options.db2Name << ':';
		logStats(newR, env);
	}
} // ColoredLitPerf::compareOne

void
ColoredLitPerf::logStats(VPResult& r, GLEAN::Environment* env) {
	logStats1("Immediate-mode independent triangle", r.imTri, env);
	logStats1("Display-listed independent triangle", r.dlTri, env);
	logStats1("DrawArrays independent triangle", r.daTri, env);
	logStats1("Locked DrawArrays independent triangle", r.ldaTri, env);
	logStats1("DrawElements independent triangle", r.deTri, env);
	logStats1("Locked DrawElements independent triangle", r.ldeTri, env);
	logStats1("Immediate-mode triangle strip", r.imTS, env);
	logStats1("Display-listed triangle strip", r.dlTS, env);
	logStats1("DrawArrays triangle strip", r.daTS, env);
	logStats1("Locked DrawArrays triangle strip", r.ldaTS, env);
	logStats1("DrawElements triangle strip", r.deTS, env);
	logStats1("Locked DrawElements triangle strip", r.ldeTS, env);
} // ColoredLitPerf::logStats

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////

Test* coloredLitPerfTestPrereqs[] = {&exactRGBATest, 0};

ColoredLitPerf coloredLitPerfTest("coloredLitPerf2", "window, rgb, z, fast",
    coloredLitPerfTestPrereqs,

	"This test examines rendering performance for colored, lit,\n"
	"flat-shaded triangles.  It checks several different ways to\n"
	"specify the vertex data in order to determine which is\n"
	"fastest:  fine-grained API calls, DrawArrays, DrawElements,\n"
	"locked (compiled) DrawArrays, and locked DrawElements; for\n"
	"independent triangles and for triangle strips.  The test\n"
	"result is performance measured in triangles per second for\n"
	"each of the various vertex specification methods.\n"

	"\nAs a sanity-check on the correctness of each method, the test\n"
	"colors each triangle with a unique color, and verifies that all\n"
	"such colors are actually present in the final image.  For\n"
	"consistency, the test also verifies that the images are identical\n"
	"for each of the specification methods.\n"

	);


///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
ColoredTexPerf::runOne(VPResult& r, Window& w) {
	// Don't bother running if the ExactRGBA test for this display
	// surface configuration failed:
	vector<ExactRGBAResult*>::const_iterator erRes;
	for (erRes = exactRGBATest.results.begin();
	    erRes != exactRGBATest.results.end();
	    ++erRes)
		if ((*erRes)->config == r.config)
			break;
	if (erRes == exactRGBATest.results.end() || !(*erRes)->ub.pass) {
		r.skipped = true;
		r.pass = false;
		return;
	}

	PFNGLLOCKARRAYSEXTPROC glLockArraysEXT = 0;
	PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT = 0;
	if (GLUtils::haveExtension("GL_EXT_compiled_vertex_array")) {
		glLockArraysEXT = reinterpret_cast<PFNGLLOCKARRAYSEXTPROC>
			(GLUtils::getProcAddress("glLockArraysEXT"));
		glUnlockArraysEXT = reinterpret_cast<PFNGLUNLOCKARRAYSEXTPROC>
			(GLUtils::getProcAddress("glUnlockArraysEXT"));
	}

	Image imTriImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);
	Image testImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);
	bool passed = true;

	// Make colors deterministic, so we can check them:
	RGBCodedID colorGen(r.config->r, r.config->g, r.config->b);
	int IDModulus = colorGen.maxID() + 1;

	// We need to minimize the number of pixels per triangle, so that
	// we're measuring vertex-processing rate rather than fill rate.
	// However, we'd also like to guarantee that every triangle covers
	// at least one pixel, so that we can confirm drawing actually took
	// place.  As a compromise, we'll choose a number of triangles that
	// yields approximately 3 pixels per triangle.
	// We're drawing a filled spiral that approximates a circular area,
	// so pi * (drawingSize/2)**2 / nTris = 3 implies...
	const int nTris = static_cast<int>
		(((3.14159 / 4.0) * drawingSize * drawingSize) / 3.0 + 0.5);
	int nVertices = nTris * 3;
	int lastID = min(IDModulus - 1, nTris - 1);

	C4UB_T2F_V3F *c4ub_t2f_v3f = new C4UB_T2F_V3F[nVertices];
	SpiralTri2D it(nTris, 0, drawingSize, 0, drawingSize);
	int k = 0;
	for (int j = 0; j < nTris; ++j) {
		float* t = it(j);
		GLubyte r, g, b;
		colorGen.toRGB(j % IDModulus, r, g, b);

		c4ub_t2f_v3f[k+0].c[0] = r;
		c4ub_t2f_v3f[k+0].c[1] = g;
		c4ub_t2f_v3f[k+0].c[2] = b;
		c4ub_t2f_v3f[k+0].c[3] = 0xFF;
		c4ub_t2f_v3f[k+0].t[0] = 0.5;
		c4ub_t2f_v3f[k+0].t[1] = 0.5;
		c4ub_t2f_v3f[k+0].v[0] = t[0];
		c4ub_t2f_v3f[k+0].v[1] = t[1];
		c4ub_t2f_v3f[k+0].v[2] = 0.0;

		c4ub_t2f_v3f[k+1].c[0] = r;
		c4ub_t2f_v3f[k+1].c[1] = g;
		c4ub_t2f_v3f[k+1].c[2] = b;
		c4ub_t2f_v3f[k+1].c[3] = 0xFF;
		c4ub_t2f_v3f[k+1].t[0] = 0.5;
		c4ub_t2f_v3f[k+1].t[1] = 0.5;
		c4ub_t2f_v3f[k+1].v[0] = t[2];
		c4ub_t2f_v3f[k+1].v[1] = t[3];
		c4ub_t2f_v3f[k+1].v[2] = 0.0;

		c4ub_t2f_v3f[k+2].c[0] = r;
		c4ub_t2f_v3f[k+2].c[1] = g;
		c4ub_t2f_v3f[k+2].c[2] = b;
		c4ub_t2f_v3f[k+2].c[3] = 0xFF;
		c4ub_t2f_v3f[k+2].t[0] = 0.5;
		c4ub_t2f_v3f[k+2].t[1] = 0.5;
		c4ub_t2f_v3f[k+2].v[0] = t[4];
		c4ub_t2f_v3f[k+2].v[1] = t[5];
		c4ub_t2f_v3f[k+2].v[2] = 0.0;

		k += 3;
	}

	GLuint *indices = new GLuint[nVertices];
	for (k = 0; k < nVertices; ++k)
		indices[k] = k;

	GLUtils::useScreenCoords(drawingSize, drawingSize);

	glFrontFace(GL_CCW);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_MATERIAL);

	glDisable(GL_LIGHTING);

	// Set up an all-white RGB texture, including mipmap levels:
	{
	const int width = 8;
	const int height = 8;
	GLubyte whiteTex[width * height * 3];
	for (int i = 0; i < width * height * 3; ++i)
		whiteTex[i] = 255;
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
	glPixelTransferf(GL_RED_SCALE, 1.0);
	glPixelTransferf(GL_GREEN_SCALE, 1.0);
	glPixelTransferf(GL_BLUE_SCALE, 1.0);
	glPixelTransferf(GL_ALPHA_SCALE, 1.0);
	glPixelTransferf(GL_RED_BIAS, 0.0);
	glPixelTransferf(GL_GREEN_BIAS, 0.0);
	glPixelTransferf(GL_BLUE_BIAS, 0.0);
	glPixelTransferf(GL_ALPHA_BIAS, 0.0);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB,
		GL_UNSIGNED_BYTE, whiteTex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glEnable(GL_TEXTURE_2D);
	}

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glReadBuffer(GL_FRONT);

	////////////////////////////////////////////////////////////
	// Immediate-mode independent triangles
	////////////////////////////////////////////////////////////
	ColoredTex_imIndTri coloredTex_imIndTri(nVertices, c4ub_t2f_v3f,
						nTris, &w, env);
	coloredTex_imIndTri.measure(5, &r.imTri.tpsLow, &r.imTri.tps,
				    &r.imTri.tpsHigh);
	imTriImage.read(0, 0);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.imTri, env,
	       "Immediate-mode independent triangle");

	////////////////////////////////////////////////////////////
	// Display-listed independent triangles
	////////////////////////////////////////////////////////////
	int dList = glGenLists(1);
	glNewList(dList, GL_COMPILE);
	coloredTex_imIndTri.op();
	glEndList();
	callDListTimer callDList(dList, nTris, &w, env);
	callDList.measure(5, &r.dlTri.tpsLow, &r.dlTri.tps, &r.dlTri.tpsHigh);
	glDeleteLists(dList, 1);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.dlTri, env,
	       "Display-listed independent triangle");

	////////////////////////////////////////////////////////////
	// DrawArrays on independent triangles
	////////////////////////////////////////////////////////////
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(c4ub_t2f_v3f[0]),
		c4ub_t2f_v3f[0].c);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(c4ub_t2f_v3f[0]),
		c4ub_t2f_v3f[0].t);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(c4ub_t2f_v3f[0]),
		c4ub_t2f_v3f[0].v);
	glEnableClientState(GL_VERTEX_ARRAY);

	daIndTriTimer daIndTri(nVertices, indices, nTris, &w, env);
	daIndTri.measure(5, &r.daTri.tpsLow, &r.daTri.tps, &r.daTri.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.daTri, env,
	       "DrawArrays independent triangle");

	////////////////////////////////////////////////////////////
	// Locked DrawArrays on independent triangles
	//	XXX This is probably unrealistically favorable to
	//	locked arrays.
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	daIndTri.measure(5, &r.ldaTri.tpsLow, &r.ldaTri.tps,
			 &r.ldaTri.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldaTri.tps = r.ldaTri.tpsLow = r.ldaTri.tpsHigh = 0.0;
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.ldaTri, env,
	       "Locked DrawArrays independent triangle");

	////////////////////////////////////////////////////////////
	// DrawElements on independent triangles
	////////////////////////////////////////////////////////////
	deIndTriTimer deIndTri(nVertices, indices, nTris, &w, env);
	deIndTri.measure(5, &r.deTri.tpsLow, &r.deTri.tps, &r.deTri.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.deTri, env,
	       "DrawElements independent triangle");

	////////////////////////////////////////////////////////////
	// Locked DrawElements on independent triangles
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	deIndTri.measure(5, &r.ldeTri.tpsLow, &r.ldeTri.tps,
			 &r.ldeTri.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldeTri.tps = r.ldeTri.tpsLow = r.ldeTri.tpsHigh = 0.0;
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.ldeTri, env,
	       "Locked DrawElements independent triangle");

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	delete[] c4ub_t2f_v3f;
	delete[] indices;

	// Now we test triangle strips, rather than independent triangles.

	nVertices = nTris + 2;
	lastID = min(IDModulus - 1, nTris - 1);

	c4ub_t2f_v3f = new C4UB_T2F_V3F[nVertices];
	SpiralStrip2D is(nVertices, 0, drawingSize, 0, drawingSize);
	for (int j2 = 0; j2 < nVertices; ++j2) {
		float* t = is(j2);
		GLubyte r, g, b;
		// Take care to get the correct color on the provoking vertex:
		colorGen.toRGB((j2 - 2) % IDModulus, r, g, b);

		c4ub_t2f_v3f[j2].c[0] = r;
		c4ub_t2f_v3f[j2].c[1] = g;
		c4ub_t2f_v3f[j2].c[2] = b;
		c4ub_t2f_v3f[j2].c[3] = 0xFF;
		c4ub_t2f_v3f[j2].t[0] = 0.5;
		c4ub_t2f_v3f[j2].t[1] = 0.5;
		c4ub_t2f_v3f[j2].v[0] = t[0];
		c4ub_t2f_v3f[j2].v[1] = t[1];
		c4ub_t2f_v3f[j2].v[2] = 0.0;
	}

	indices = new GLuint[nVertices];
	for (int j3 = 0; j3 < nVertices; ++j3)
		indices[j3] = j3;

	////////////////////////////////////////////////////////////
	// Immediate-mode triangle strips
	////////////////////////////////////////////////////////////
	ColoredTex_imTriStrip coloredTex_imTriStrip(nVertices, c4ub_t2f_v3f,
						    nTris, &w, env);
	coloredTex_imTriStrip.measure(5, &r.imTS.tpsLow, &r.imTS.tps,
				      &r.imTS.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.imTS, env,
	       "Immediate-mode triangle strip");

	////////////////////////////////////////////////////////////
	// Display-listed triangle strips
	////////////////////////////////////////////////////////////
	dList = glGenLists(1);
	glNewList(dList, GL_COMPILE);
	coloredTex_imTriStrip.op();
	glEndList();
	callDList.dList = dList;
	callDList.measure(5, &r.dlTS.tpsLow, &r.dlTS.tps, &r.dlTS.tpsHigh);
	glDeleteLists(dList, 1);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.dlTS, env,
	       "Display-listed triangle strip");

	////////////////////////////////////////////////////////////
	// DrawArrays on triangle strips
	////////////////////////////////////////////////////////////
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(c4ub_t2f_v3f[0]),
		c4ub_t2f_v3f[0].c);
	glEnableClientState(GL_COLOR_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(c4ub_t2f_v3f[0]),
		c4ub_t2f_v3f[0].t);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(c4ub_t2f_v3f[0]),
		c4ub_t2f_v3f[0].v);
	glEnableClientState(GL_VERTEX_ARRAY);

	daTriStripTimer daTriStrip(nVertices, nTris, &w, env);
	daTriStrip.measure(5, &r.daTS.tpsLow, &r.daTS.tps, &r.daTS.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.daTS, env,
	       "DrawArrays triangle strip");

	////////////////////////////////////////////////////////////
	// Locked DrawArrays on triangle strips
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	daTriStrip.measure(5, &r.ldaTS.tpsLow, &r.ldaTS.tps, &r.ldaTS.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldaTS.tps = r.ldaTS.tpsLow = r.ldaTS.tpsHigh = 0.0;
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.ldaTS, env,
		"Locked DrawArrays triangle strip");

	////////////////////////////////////////////////////////////
	// DrawElements on triangle strips
	////////////////////////////////////////////////////////////
	deTriStripTimer deTriStrip(nVertices, indices, nTris, &w, env);
	deTriStrip.measure(5, &r.deTS.tpsLow, &r.deTS.tps, &r.deTS.tpsHigh);
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.deTS, env,
	       "DrawElements triangle strip");

	////////////////////////////////////////////////////////////
	// Locked DrawElements on triangle strips
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	deTriStrip.measure(5, &r.ldeTS.tpsLow, &r.ldeTS.tps, &r.ldeTS.tpsHigh);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldeTS.tps = r.ldeTS.tpsLow = r.ldeTS.tpsHigh = 0.0;
	verifyVtxPerf(testImage, colorGen, 0, lastID, imTriImage,
	       passed, name, r.config, r.ldeTS, env,
	       "Locked DrawElements triangle strip");


	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	delete[] c4ub_t2f_v3f;
	delete[] indices;

	r.pass = passed;
	r.skipped = false;
} // ColoredTexPerf::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
ColoredTexPerf::logOne(VPResult& r) {
	if (r.skipped) {
		env->log << name << ":  NOTE ";
		logConcise(r);
		env->log << "\tTest skipped; prerequisite test "
			 << exactRGBATest.name
			 << " failed or was not run\n"
			 ;
		return;
	}
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	} else env->log << '\n'; // because verify logs failure
	logStats(r, env);
} // ColoredTexPerf::logOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
ColoredTexPerf::compareOne(VPResult& oldR, VPResult& newR) {
	if (oldR.skipped || newR.skipped) {
		env->log << name
			 << ((oldR.skipped && newR.skipped)? ":  SAME "
			 	: ":  DIFF ")
			 << newR.config->conciseDescription()
			 << '\n';
		if (oldR.skipped)
			 env->log << "\t"
				  << env->options.db1Name
				  << " skipped\n";
		if (newR.skipped)
			 env->log << "\t"
				  << env->options.db2Name
				  << " skipped\n";
		env->log << "\tNo comparison is possible.\n";
		return;
	}

	bool same = true;
	doComparison(oldR.imTri, newR.imTri, newR.config, same, name,
		env, "immediate-mode independent triangle");
	doComparison(oldR.dlTri, newR.dlTri, newR.config, same, name,
		env, "display-listed independent triangle");
	doComparison(oldR.daTri, newR.daTri, newR.config, same, name,
		env, "DrawArrays independent triangle");
	doComparison(oldR.ldaTri, newR.ldaTri, newR.config, same, name,
		env, "Locked DrawArrays independent triangle");
	doComparison(oldR.deTri, newR.deTri, newR.config, same, name,
		env, "DrawElements independent triangle");
	doComparison(oldR.ldeTri, newR.ldeTri, newR.config, same, name,
		env, "Locked DrawElements independent triangle");
	doComparison(oldR.imTS, newR.imTS, newR.config, same, name,
		env, "immediate-mode triangle strip");
	doComparison(oldR.dlTS, newR.dlTS, newR.config, same, name,
		env, "display-listed triangle strip");
	doComparison(oldR.daTS, newR.daTS, newR.config, same, name,
		env, "DrawArrays triangle strip");
	doComparison(oldR.ldaTS, newR.ldaTS, newR.config, same, name,
		env, "Locked DrawArrays triangle strip");
	doComparison(oldR.deTS, newR.deTS, newR.config, same, name,
		env, "DrawElements triangle strip");
	doComparison(oldR.ldeTS, newR.ldeTS, newR.config, same, name,
		env, "Locked DrawElements triangle strip");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n\t"
			<< env->options.db2Name
			<< " test time falls within the "
			<< "valid measurement range of\n\t"
			<< env->options.db1Name
			<< " test time; both have the same"
			<< " image comparison results.\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR, env);
		env->log << env->options.db2Name << ':';
		logStats(newR, env);
	}
} // ColoredTexPerf::compareOne

void
ColoredTexPerf::logStats(VPResult& r, GLEAN::Environment* env) {
	logStats1("Immediate-mode independent triangle", r.imTri, env);
	logStats1("Display-listed independent triangle", r.dlTri, env);
	logStats1("DrawArrays independent triangle", r.daTri, env);
	logStats1("Locked DrawArrays independent triangle", r.ldaTri, env);
	logStats1("DrawElements independent triangle", r.deTri, env);
	logStats1("Locked DrawElements independent triangle", r.ldeTri, env);
	logStats1("Immediate-mode triangle strip", r.imTS, env);
	logStats1("Display-listed triangle strip", r.dlTS, env);
	logStats1("DrawArrays triangle strip", r.daTS, env);
	logStats1("Locked DrawArrays triangle strip", r.ldaTS, env);
	logStats1("DrawElements triangle strip", r.deTS, env);
	logStats1("Locked DrawElements triangle strip", r.ldeTS, env);
} // ColoredTexPerf::logStats

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
//
Test* coloredTexPerfTestPrereqs[] = {&exactRGBATest, 0};

ColoredTexPerf coloredTexPerfTest("coloredTexPerf2", "window, rgb, z, fast",
    coloredTexPerfTestPrereqs,

	"This test examines rendering performance for colored, textured,\n"
	"flat-shaded triangles.  It checks several different ways to\n"
	"specify the vertex data in order to determine which is\n"
	"fastest:  fine-grained API calls, DrawArrays, DrawElements,\n"
	"locked (compiled) DrawArrays, and locked DrawElements; for\n"
	"independent triangles and for triangle strips.  The test\n"
	"result is performance measured in triangles per second for\n"
	"each of the various vertex specification methods.\n"

	"\nAs a sanity-check on the correctness of each method, the test\n"
	"colors each triangle with a unique color, and verifies that all\n"
	"such colors are actually present in the final image.  For\n"
	"consistency, the test also verifies that the images are identical\n"
	"for each of the specification methods.\n"

	);

} // namespace GLEAN
