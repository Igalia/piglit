// BEGIN_COPYRIGHT -*- glean -*-

/*
 * Copyright © 2006 Intel Corporation
 * Copyright © 1999 Allen Akin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *  Wei Wang <wei.z.wang@intel.com>
 *
 */

/* 
 * toccluqry.cpp: Conformance test on ARB_occlusion_query extension
 */

#define GL_GLEXT_PROTOTYPES
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <cmath>
#include "toccluqry.h"


#define START_QUERY(id)\
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);


#define TERM_QUERY()\
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);\


namespace GLEAN {

/* Generate a box which will be occluded by the occluder */
void OccluQryTest::gen_box(GLfloat left, GLfloat right,
			  GLfloat top, GLfloat btm)
{
	glBegin(GL_POLYGON);
	glVertex3f(left, top, 0);
	glVertex3f(right, top, 0);
	glVertex3f(right, btm, 0);
	glVertex3f(left,  btm, 0);
	glEnd();
}


bool OccluQryTest::chk_ext()
{
	const char *ext = (const char *) glGetString(GL_EXTENSIONS);

	if (!strstr(ext, "GL_ARB_occlusion_query")) {
		fprintf(stdout, "W: Extension GL_ARB_occlusion_query is missing.\n");
		return false;
	}

	return true;
}


GLuint OccluQryTest::find_unused_id()
{
	unsigned int id;
	int counter = 0;

#define MAX_FIND_ID_ROUND 256

	while (1) {
		/* assuming that at least 2^32-1 <id> can be generated */
		id = random() % ((unsigned int)-1);
		if (id != 0 && glIsQueryARB(id) == GL_FALSE)
			return id;
		if (++ counter >= MAX_FIND_ID_ROUND) {
			fprintf(stderr, 
				"W: Cannot find the unused id after [%d] tries.\n",
				MAX_FIND_ID_ROUND);
			return 0;
		}
	}
}


/* If multiple queries are issued on the same target and id prior to calling
 * GetQueryObject[u]iVARB, the result returned will always be from the last
 * query issued.  The results from any queries before the last one will be lost
 * if the results are not retrieved before starting a new query on the same
 * target and id.
 */
bool OccluQryTest::conformOQ_GetObjivAval_multi1(GLuint id)
{
	GLint ready;
	GLuint passed = 0;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( -1.0, 1.0, -1.0, 1.0, 0.0, 25.0 );
	
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -10.0);

	/* draw the occluder (red) */
	glColorMask(1, 1, 1, 1);
	glDepthMask(GL_TRUE);
	glColor3f(1, 0, 0);
	gen_box(-0.5, 0.5, 0.5, -0.5);

	glPushMatrix();
	glTranslatef( 0.0, 0.0, -5.0);
	glColorMask(0, 0, 0, 0);
	glDepthMask(GL_FALSE);

	/* draw the 1st box (gren) which is occluded by the occluder partly */
	START_QUERY(id);
	glColor3f(0, 1, 0);
	gen_box(-0.51,  0.51, 0.51, -0.51);
	TERM_QUERY();

	/* draw the 2nd box (blue) which is occluded by the occluder throughly */
	START_QUERY(id);
	glColor3f(0, 0, 1);
	gen_box(-0.4, 0.4, 0.4, -0.4); 
	TERM_QUERY();

	glPopMatrix();

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	
	do {
		glGetQueryObjectivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB, &ready);
	} while (!ready);
	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_ARB, &passed);

	// 'passed' should be zero
	return passed > 0 ? false : true;
}


/* If mutiple queries are issued on the same target and diff ids prior
 * to calling GetQueryObject[u]iVARB, the results should be
 * corresponding to those queries (ids) respectively.
 */
bool OccluQryTest::conformOQ_GetObjivAval_multi2()
{
	GLuint passed1 = 0, passed2 = 0, passed3 = 0;
	GLuint id1, id2, id3;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( -1.0, 1.0, -1.0, 1.0, 0.0, 25.0 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -10.0);


	/* draw the occluder (red) */
	glColorMask(1, 1, 1, 1);
	glDepthMask(GL_TRUE);
	glColor3f(1, 0, 0);
	gen_box(-0.5, 0.5, 0.5, -0.5);

	glPushMatrix();
	glTranslatef( 0.0, 0.0, -5.0);
	glColorMask(0, 0, 0, 0);
	glDepthMask(GL_FALSE);

	id1 = find_unused_id();
	START_QUERY(id1);
	/* draw green quad, much larger than occluder */
	glColor3f(0, 1, 0);
	gen_box(-0.7, 0.7, 0.7, -0.7);
	TERM_QUERY();

	id2 = find_unused_id();
	START_QUERY(id2);
	/* draw blue quad, slightly larger than occluder */
	glColor3f(0, 0, 1);
	gen_box(-0.53, 0.53, 0.53, -0.53);
	TERM_QUERY();

	id3 = find_unused_id();
	START_QUERY(id3);
	/* draw white quad, smaller than occluder (should not be visible) */
	glColor3f(1, 1, 1);
	gen_box(-0.4, 0.4, 0.4, -0.4);
	TERM_QUERY();

	glPopMatrix();

	glGetQueryObjectuivARB(id1, GL_QUERY_RESULT_ARB, &passed1);
	glGetQueryObjectuivARB(id2, GL_QUERY_RESULT_ARB, &passed2);
	glGetQueryObjectuivARB(id3, GL_QUERY_RESULT_ARB, &passed3);

	glDepthMask(GL_TRUE);

	
	glDeleteQueriesARB(1, &id1);
	glDeleteQueriesARB(1, &id2);
	glDeleteQueriesARB(1, &id3);

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	
	if ( passed1 > passed2 && passed2 > passed3 && passed3 == 0)
		return true;
	else
		return false;
}


/* 
 * void GetQueryivARB(enum target, enum pname, int *params);
 *
 * If <pname> is QUERY_COUNTER_BITS_ARB, the number of bits in the counter
 * for <target> will be placed in <params>.  The minimum number of query
 * counter bits allowed is a function of the implementation's maximum
 * viewport dimensions (MAX_VIEWPORT_DIMS).  If the counter is non-zero,
 * then the counter must be able to represent at least two overdraws for
 * every pixel in the viewport using only one sample buffer.  The formula to
 * compute the allowable minimum value is below (where n is the minimum
 * number of bits): 
 *   n = (min (32, ceil (log2 (maxViewportWidth x maxViewportHeight x 2) ) ) ) or 0
 */
bool OccluQryTest::conformOQ_GetQry_CnterBit()
{
	int bit_num, dims[2];
	GLenum err;
	float min_impl, min_bit_num;

	/* get the minimum bit number supported by the implementation, 
	 * and check the legality of result of GL_QUERY_COUNTER_BITS_ARB */
	glGetQueryivARB(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bit_num);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
	err = glGetError();
	if (err == GL_INVALID_OPERATION || err == GL_INVALID_ENUM) 
		return false;

	min_impl = ceil(logf((float)dims[0]*(float)dims[1]*1.0*2.0) / logf(2.0));
	min_bit_num = 32.0 > min_impl ? min_impl : 32.0;

	if ((float)bit_num < min_bit_num)
		return false;

	return true;
}


/* If BeginQueryARB is called with an unused <id>, that name is marked as used
 * and associated with a new query object. */
bool OccluQryTest::conformOQ_Begin_unused_id()
{
	unsigned int id;
	bool pass = true;

	id = find_unused_id();

	if (id == 0)
		return false;

	glBeginQuery(GL_SAMPLES_PASSED_ARB, id);

	if (glIsQueryARB(id) == GL_FALSE) {
		fprintf(stderr, "F: Begin with a unused id failed.\n");
		pass = false;
	}

	glEndQuery(GL_SAMPLES_PASSED_ARB);

	return pass;
}

/* if EndQueryARB is called while no query with the same target is in progress,
 * an INVALID_OPERATION error is generated. */
bool OccluQryTest::conformOQ_EndAfter(GLuint id)
{
	START_QUERY(id);
	TERM_QUERY();  

	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	if (glGetError() != GL_INVALID_OPERATION) {
		fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
			"EndQuery when there is no queries.\n");
		return false;
	}

	return true;
}


/* Calling either GenQueriesARB while any query of any target is active causes
 * an INVALID_OPERATION error to be generated. */
bool OccluQryTest::conformOQ_GenIn(GLuint id)
{
	int pass = true;

	START_QUERY(id);

	glGenQueriesARB(1, &id);
	if (glGetError() != GL_INVALID_OPERATION) {
		fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
			"GenQueries in the progress of another.\n");
		pass = false;
	}
  
	TERM_QUERY();

	return pass;
}


/* Calling either DeleteQueriesARB while any query of any target is active causes
 * an INVALID_OPERATION error to be generated. */
bool OccluQryTest::conformOQ_DeleteIn(GLuint id)
{
	int pass = true;

	START_QUERY(id);

	if (id > 0) {
		glDeleteQueriesARB(1, &id);

		if (glGetError() != GL_INVALID_OPERATION) {
			fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
				"DeleteQueries in the progress of another.\n");
			pass = false;
		}
	}
  
	TERM_QUERY();

	return pass;
}


/* If BeginQueryARB is called while another query is already in progress with
 * the same target, an INVALID_OPERATION error should be generated. */
bool OccluQryTest::conformOQ_BeginIn(GLuint id)
{
	int pass = true;

	START_QUERY(id);

	/* Issue another BeginQueryARB while another query is already in 
	   progress */
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);

	if (glGetError() != GL_INVALID_OPERATION) {
		fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
			"BeginQuery in the progress of another.\n");
		pass = false;
	}

	TERM_QUERY();
	return pass;
}


/* if the query object named by <id> is currently active, then an
 * INVALID_OPERATION error is generated. */
bool OccluQryTest::conformOQ_GetObjAvalIn(GLuint id)
{
	int pass = true;
	GLint param;

	START_QUERY(id);

	glGetQueryObjectivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB, &param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB, (GLuint *)&param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	if (pass == false) {
		fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
			"GetQueryObjectuiv with GL_QUERY_RESULT_AVAILABLE_ARB "
			"in the active progress.\n");
	}
	TERM_QUERY();

	return pass;
}


/* if the query object named by <id> is currently active, then an
 * INVALID_OPERATION error is generated. */
bool OccluQryTest::conformOQ_GetObjResultIn(GLuint id)
{
	int pass = true;
	GLint param;

	START_QUERY(id);

	glGetQueryObjectivARB(id, GL_QUERY_RESULT_ARB, &param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_ARB, (GLuint *)&param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	if (pass == false) {
		fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
			"GetQueryObject[u]iv with GL_QUERY_RESULT_ARB "
			"in the active progress.\n");
	}
	TERM_QUERY();

	return pass;
}


/* If <id> is not the name of a query object, then an INVALID_OPERATION error
 * is generated. */
bool OccluQryTest::conformOQ_GetObjivAval(GLuint id)
{
	GLuint id_tmp;
	GLint param;
	
	START_QUERY(id);
	TERM_QUERY();
	
	id_tmp = find_unused_id();
 
	if (id_tmp == 0)
		return false;

	glGetQueryObjectivARB(id_tmp, GL_QUERY_RESULT_AVAILABLE_ARB, &param);

	if (glGetError() != GL_INVALID_OPERATION) {
		fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
			"GetQueryObjectuiv can still query the result"
			"by an unused query id\n.");
		return false;
	}

	return true;
}


/* Basic tests on query id generation and deletion */
bool OccluQryTest::conformOQ_Gen_Delete(unsigned int id_n)
{
	GLuint *ids1 = NULL, *ids2 = NULL;
	unsigned int i, j;
	bool pass = true;

	ids1 = (GLuint *)malloc(id_n * sizeof(GLuint));
	ids2 = (GLuint *)malloc(id_n * sizeof(GLuint));

	if (!ids1 || !ids2) {
		fprintf(stderr, "F: Cannot alloc memory to pointer ids[12].\n");
		return false;
	}

	glGenQueriesARB(id_n, ids1);
	glGenQueriesARB(id_n, ids2);

	/* compare whether <id> generated during the previous 2 rounds are
	 * duplicated */
	for (i = 0; i < id_n; i ++) {
		for (j = 0; j < id_n; j ++) {
			if (ids1[i] == ids2[j]) {
				fprintf(stderr, "F: ids1[%d] == ids2[%d] == %u.\n", 
						i, j, ids1[i]);
				pass = false;
			}
		}
	}

	/* Note: the spec seems to indicate that glGenQueries reserves query
	 * IDs but doesn't create query objects for those IDs.  A query object
	 * isn't created until they are used by glBeginQuery.  So this part
	 * of the test is invalid.
	 */
#if 0
	/* Checkout whether the Query ID just generated is valid */	
	for (i = 0; i < id_n; i ++) {
		if (glIsQueryARB(ids1[i]) == GL_FALSE) {
			fprintf(stderr, "F: id [%d] just generated is not valid.\n", 
				ids1[i]);
			pass = false;
		}
	}
#endif

	/* if <id> is a non-zero value that is not the name of a query object,
	 * IsQueryARB returns FALSE. */
	glDeleteQueriesARB(id_n, ids1);
	for (i = 0; i < id_n; i ++) {
		if (glIsQueryARB(ids1[i]) == GL_TRUE) {
			fprintf(stderr, "F: id [%d] just deleted is still valid.\n", 
				ids1[i]);
			pass = false;
		}
	}

	/* Delete only for sanity purpose */
	glDeleteQueriesARB(id_n, ids2);

	if (ids1)
		free(ids1);
	if (ids2)
		free(ids2);


	ids1 = (GLuint *)malloc(id_n * sizeof(GLuint));
	if (ids1 == NULL)
		return false;

	for (i = 0; i < id_n; i ++) {
		glGenQueriesARB(1, ids1 + i);
		for (j = 0; j < i; j ++) {
			if (ids1[i] == ids1[j]) {
				fprintf(stderr, "E: duplicated id generated [ %u ]", 
					ids1[i]);
				pass = false;
			}
		}
	}

	glDeleteQueriesARB(id_n, ids1);
	if (ids1)
		free(ids1);

	return pass;
}


/* If <id> is zero, IsQueryARB should return FALSE.*/
bool OccluQryTest::conformOQ_IsIdZero(void)
{
	if (glIsQueryARB(0) == GL_TRUE) {
		fprintf(stderr, "F: zero is treated as a valid id by"
				"IsQueryARB().\n");
		return false;
	}
	
	return true;
}


/* If BeginQueryARB is called with an <id> of zero, an INVALID_OPERATION error
 * should be generated. */
bool OccluQryTest::conformOQ_BeginIdZero(void)
{
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, 0);
	if (glGetError() != GL_INVALID_OPERATION) {
		fprintf(stderr, "F: No GL_INVALID_OPERATION generated if "
				"BeginQuery with zero ID.\n");
		return false;
	}

	return true;
}


void OccluQryTest::runOne(MultiTestResult &r, Window &w)
{
	bool result;
	(void) w;
	GLuint queryId;

	if (!chk_ext())
		return;
	glEnable(GL_DEPTH_TEST);
	glGenQueriesARB(1, &queryId);

	if (queryId == 0)
		return;

#if defined(GL_ARB_occlusion_query)
	result = conformOQ_GetQry_CnterBit();
	reportPassFail(r, result, "conformOQ_GetQry_CnterBit");

	result = conformOQ_GetObjivAval_multi1(queryId);
	reportPassFail(r, result, "conformOQ_GetObjivAval_multi1");

	result = conformOQ_GetObjivAval_multi2();
	reportPassFail(r, result, "conformOQ_GetObjivAval_multi2");

	result = conformOQ_Begin_unused_id();
	reportPassFail(r, result, "conformOQ_Begin_unused_id");

	result = conformOQ_EndAfter(queryId);
	reportPassFail(r, result, "conformOQ_EndAfter");

	result = conformOQ_GenIn(queryId);
	reportPassFail(r, result, "conformOQ_GenIn");

	result = conformOQ_BeginIn(queryId);
	reportPassFail(r, result, "conformOQ_BeginIn");

	result = conformOQ_DeleteIn(queryId);
	reportPassFail(r, result, "conformOQ_DeleteIn");

	result = conformOQ_GetObjAvalIn(queryId);
	reportPassFail(r, result, "conformOQ_GetObjAvalIn");

	result = conformOQ_GetObjResultIn(queryId);
	reportPassFail(r, result, "conformOQ_GetObjResultIn");

	result = conformOQ_GetObjivAval(queryId);
	reportPassFail(r, result, "conformOQ_GetObjivAval");

	result = conformOQ_Gen_Delete(64);
	reportPassFail(r, result, "conformOQ_Gen_Delete");

	result = conformOQ_IsIdZero();
	reportPassFail(r, result, "conformOQ_IsIdZero");

	result = conformOQ_BeginIdZero();
	reportPassFail(r, result, "conformOQ_BeginIdZero");

	glDeleteQueriesARB(1, &queryId);

	r.pass = (r.numFailed == 0);
#endif
}


void OccluQryTest::reportPassFail(MultiTestResult &r,
				  bool pass, const char *msg) const
{
	if (pass) {
		if (env->options.verbosity)
			env->log << name << " subcase PASS: "
				 << msg << " test\n";
		r.numPassed++;
	} else {
		if (env->options.verbosity)
			env->log << name << " subcase FAIL: "
				 << msg << " test\n";
		r.numFailed++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
OccluQryTest occluQryTest("occluQry", "window, rgb, z",
			"GL_ARB_occlusion_query",
			"Test occlusion query comformance.\n");
} // namespace GLEAN
