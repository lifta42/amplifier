/**
 * \file sand.h
 * \brief A Simple cross-platform subprocess manager.
 * \author liftA42
 * \date Dec 14, 2017
 *
 * Run something dangerous with a sandbox. The failure and modify will affect
 * main process like multithread does.
 */
#ifndef LIFTA42_NAIVE_SAND_H
#define LIFTA42_NAIVE_SAND_H

typedef struct _Sandbox *Sandbox;

/** The callback acts as a subprocess.

Accept an optional payload to capture objects from parent.
The result status code can either be returned or be passed to `exit`. */
typedef int(SandProc)(void *);

/** Create a new sandbox.

\param proc the work of sandbox's subprocess
\return The created sandbox, not started. */
Sandbox sand_create(SandProc *proc);

/** Run a sandbox and wait for it to stop.

\param the executed sandbox
\param payload the optional extra object that passes into subprocess
\return The status of executing. */
int sand_run(Sandbox box, void *payload);

/** Read sandbox's output.

\param box the read sandbox
\param stream which stream to read, 0 for stdout, 1 for stderr
\param buffer where to write the read content
\param buffer_size the size of `buffer`
\return The actual bytes written into `buffer`. */
int sand_read(Sandbox box, int stream, char *buffer, int buffer_size);

/** Release a sandbox.

\param box the released sandbox */
void sand_destroy(Sandbox box);

#endif
