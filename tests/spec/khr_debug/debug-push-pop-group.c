/*
 * Copyright (c) 2013 Timothy Arceri <t_arceri@yahoo.com.au>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"

static const char *TestMessage1 = "Piglit Message 1";
static const char *TestMessage2 = "Piglit Message 2";
static const char *TestMessage3 = "Piglit Message 3";
static const char *TestMessage4 = "Piglit Message 4";

static const int MessageId1 = 101;
static const int MessageId2 = 202;
static const int MessageId3 = 303;
static const int MessageId4 = 404;

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 11;
	config.require_debug_context = true;
#else /* using GLES */
	config.supports_gl_es_version = 20;
#endif

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#ifdef PIGLIT_USE_OPENGL
#define GET_FUNC(x) x
#else /* using GLES */
#define GET_FUNC(x) x ## KHR
#endif

static PFNGLGETDEBUGMESSAGELOGPROC GetDebugMessageLog;
static PFNGLDEBUGMESSAGEINSERTPROC DebugMessageInsert;
static PFNGLDEBUGMESSAGECONTROLPROC DebugMessageControl;
static PFNGLPUSHDEBUGGROUPPROC PushDebugGroup;
static PFNGLPOPDEBUGGROUPPROC PopDebugGroup;

static GLboolean fetch_one_log_message()
{
	char log[4096];
	GLboolean ret =
		!!GetDebugMessageLog(1, 4096, NULL, NULL, NULL, NULL, NULL, log);

	if (ret) {
		printf("Log: %s\n", log);
	}
	return ret;
}

static bool check_inheritance_messages(int expectedCount, GLuint* expectedIds)
{
	bool pass = true;
#define MAX_MESSAGES 5
#define BUF_SIZE 1280
	int i;
	GLuint count;
	GLuint ids[MAX_MESSAGES];
	GLchar messageLog[BUF_SIZE];

	count = GetDebugMessageLog(MAX_MESSAGES,
				     BUF_SIZE,
				     NULL,
				     NULL,
				     ids,
				     NULL,
				     NULL,
				     messageLog);

	if (count != expectedCount) {
		fprintf(stderr, "Expected message count: %i Actual message count: %i\n",
		        expectedCount, count);
		pass = false;
	} else {
		for (i = 0; i < expectedCount; i++) {
			if (expectedIds[i] != ids[i]) {
				fprintf(stderr, "Expected id: %i Actual id: %i\n",
				        expectedIds[i], ids[i]);
				pass = false;
			}
		}
	}

	return pass;
}

static void insert_inheritance_messages()
{
	DebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, MessageId1,
			     GL_DEBUG_SEVERITY_NOTIFICATION, -1, TestMessage1);

	DebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, MessageId2,
			     GL_DEBUG_SEVERITY_NOTIFICATION, -1, TestMessage2);

	DebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, MessageId3,
			     GL_DEBUG_SEVERITY_NOTIFICATION, -1, TestMessage3);

	DebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, MessageId4,
			     GL_DEBUG_SEVERITY_NOTIFICATION, -1, TestMessage4);
}

/*
 * Test inheritance of group filtering (nesting)
 */
static bool test_push_pop_group_inheritance()
{
	bool pass = true;
	GLuint allowedIds1[] = {MessageId1};
	GLuint allowedIds2[] = {MessageId2};
	GLuint allowedIds3[] = {MessageId3};

	GLuint expectedIds1[] = {MessageId1};
	GLuint expectedIds2[] = {MessageId1, MessageId2};
	GLuint expectedIds3[] = {MessageId1, MessageId2, MessageId3};

	puts("Testing Push debug group inheritance");

	/* Setup of the default active debug group: Filter everything out */
	DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
			      GL_DONT_CARE, 0, NULL, GL_FALSE);

	/* Push debug group 1 and allow messages with the id 101*/
	PushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Push_Pop 1");
	DebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER,
			      GL_DONT_CARE, 1, allowedIds1, GL_TRUE);
	insert_inheritance_messages();
	pass = check_inheritance_messages(1, expectedIds1);

	/* Push debug group 1 and allow messages with the id 101 and 202*/
	PushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Push_Pop 2");
	DebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER,
			      GL_DONT_CARE, 1, allowedIds2, GL_TRUE);
	insert_inheritance_messages();
	pass = check_inheritance_messages(2, expectedIds2) && pass;

	/* Push debug group 1 and allow messages with the id 101, 202 and 303*/
	PushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Push_Pop 3");
	DebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER,
			      GL_DONT_CARE, 1, allowedIds3, GL_TRUE);
	insert_inheritance_messages();
	pass = check_inheritance_messages(3, expectedIds3) && pass;

	puts("Testing Pop debug group inheritance");

	/* Pop debug group 3 */
	PopDebugGroup();
	insert_inheritance_messages();
	pass = check_inheritance_messages(2, expectedIds2) && pass;

	/* Pop debug group 2 */
	PopDebugGroup();
	insert_inheritance_messages();
	pass = check_inheritance_messages(1, expectedIds1) && pass;

	/* Pop group 1, restore the volume control of the default debug group. */
	PopDebugGroup();
	insert_inheritance_messages();
	/* check message log is empty, all messages should have been filtered */
	if (fetch_one_log_message()) {
		fprintf(stderr, "The message log should be empty\n");
		pass = false;
	}

	return pass;
}

/*
 * Test Push/Pop Debug Group
 */
static bool test_push_pop_debug_group()
{
	bool pass = true;
#define MAX_MESSAGES 5
#define BUF_SIZE 1280
	int i, nextMessage = 0;
	int messageLen;
	GLuint count;
	GLint maxMessageLength;
	GLint maxMessageLogLength;

	GLsizei lengths[MAX_MESSAGES];
	GLchar messageLog[BUF_SIZE];

	/* Make sure the implementation has max values big enough to run this test
	 * since the spec only mandates GL_MAX_DEBUG_MESSAGE_LENGTH and
	 * GL_MAX_DEBUG_LOGGED_MESSAGES to be 1 or larger.
	 */
	glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMessageLength);
	glGetIntegerv(GL_MAX_DEBUG_LOGGED_MESSAGES, &maxMessageLogLength);
	/* assume all test messages are of the same length */
	messageLen = strlen(TestMessage1);
	/* MAX_DEBUG_MESSAGE_LENGTH must be greater than messageLen as it includes the null terminator */
	if (maxMessageLength <= messageLen) {
		printf("push_pop_debug_group test skipped implementations MAX_DEBUG_MESSAGE_LENGTH=%i and max piglit test length=%i\n", maxMessageLength, messageLen);
		return pass;
	}
	if (maxMessageLogLength < MAX_MESSAGES) {
		printf("push_pop_debug_group test skipped implementations MAX_DEBUG_LOGGED_MESSAGES=%i and max piglit test length=%i\n", maxMessageLogLength, MAX_MESSAGES);
		return pass;
	}

	puts("Testing Push Pop debug message group");

	/* Setup of the default active debug group, only enabling
	 * the messages we will be interested in.
	 */
	DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
			      GL_DONT_CARE, 0, NULL, GL_FALSE);
	DebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PUSH_GROUP,
			      GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);
	DebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_POP_GROUP,
			      GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);
	DebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER,
			      GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);

	/* Generate a debug marker debug output message */
	DebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, MessageId1,
			     GL_DEBUG_SEVERITY_NOTIFICATION, -1, TestMessage1);

	/* Push debug group 1 */
	PushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, TestMessage2);

	/* Setup of the debug group 1: Filter everything out */
	DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
			      0, NULL, GL_FALSE);

	/* This message shouldn't appear in the debug output log */
	DebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, MessageId1,
			     GL_DEBUG_SEVERITY_NOTIFICATION, -1, TestMessage3);

	/* Pop group 1, restore the volume control of the default debug group. */
	PopDebugGroup();

	/* Generate a debug marker debug output message */
	DebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, MessageId1,
			     GL_DEBUG_SEVERITY_NOTIFICATION, -1, TestMessage4);

	/* Check that message log has done correct filtering */
	count = GetDebugMessageLog(MAX_MESSAGES,
				     BUF_SIZE,
				     NULL,
				     NULL,
				     NULL,
				     NULL,
				     lengths,
				     messageLog);

	if (count != 4) {
		fprintf(stderr, "The message log should contain 4 messages not %i messages\n", count);
		nextMessage = 0;
		for (i = 0; i < count; i++) {
			fprintf(stderr, "%s\n", messageLog+nextMessage);
			nextMessage += lengths[i];
		}
		pass = false;
	}

	if (pass) {
		/* the third message should contain TestMessage2 from PopDebugGroup() */
		nextMessage = lengths[0] + lengths[1];
		if (strstr(messageLog+nextMessage, TestMessage2) == NULL) {
			fprintf(stderr, "Expected: %s Message: %s\n", TestMessage2, messageLog+nextMessage);
			pass = false;
		}

		/* double check that TestMessage3 didnt sneak into the log */
		nextMessage = 0;
		for (i = 0; i < count; i++) {
			if (strstr(messageLog+nextMessage, TestMessage3) != NULL) {
				fprintf(stderr, "The log should not contain the message: %s",
					messageLog+nextMessage);
				pass = false;
			}
			nextMessage += lengths[i];
		}

		/* the forth message should contain TestMessage4 */
		nextMessage = lengths[0] + lengths[1] + lengths[2];
		if (strstr(messageLog+nextMessage, TestMessage4) == NULL) {
			fprintf(stderr, "Expected: %s Message: %s\n", TestMessage4, messageLog+nextMessage);
			pass = false;
		}
	}

	return pass;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;

	GetDebugMessageLog = GET_FUNC(glGetDebugMessageLog);
	DebugMessageInsert = GET_FUNC(glDebugMessageInsert);
	DebugMessageControl = GET_FUNC(glDebugMessageControl);
	PushDebugGroup = GET_FUNC(glPushDebugGroup);
	PopDebugGroup = GET_FUNC(glPopDebugGroup);

	piglit_require_extension("GL_KHR_debug");

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* clear_message_log */
	while(fetch_one_log_message())
		/* empty */ ;

	/* test message control and debug groups */
	pass = test_push_pop_debug_group();
	pass = test_push_pop_group_inheritance() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}
