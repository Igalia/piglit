#define TEST_HEIGHT 10
#define TEST_WIDTH 10

/* We're going to be emitting a TRISTRIP to form a square (after doing a
 * clear). This makes our pipeline quite predictable. */
#define NUM_VERTS 4
#define NUM_PRIMS 2

struct query {
	GLuint obj;
	GLuint query;
	const char *name;
	GLuint64 expected;
};

static inline void
begin_query(const struct query *q)
{
	glBeginQuery(q->query, q->obj);
}

static inline void
end_query(const struct query *q)
{
	glEndQuery(q->query);
}

void do_query_init(struct query *queries, const int count);
enum piglit_result do_query(const struct query *queries, const int count);
