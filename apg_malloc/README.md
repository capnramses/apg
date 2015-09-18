#apg_malloc#

An experimental memory accounting utility.
I intend to wrap `malloc()` and `free()`, providing some accounting information.
Other libraries to do this exist - this is more of a personal project to try
some slightly different ideas. I've been plotting this for a while, since my
last game debugging stint.

Today I saw [stb_leakcheck](https://github.com/nothings/stb/blob/master/stb_leakcheck.h),
which, although didn't quite work for me, validated some of my ponderings.
I decided to emulate the implementation of that, with some changes so that it
worked nicely with my coding preferences.

I stuck with a really little memory accounting structure, rather
than an 'glue to the end of the block' version to reduce
errors/issues/segfaults which I was getting pretty tired of. Probably due to
hard-coded memory distances somewhere in my game. Anyway, this limits us to
small projects of ~hundreds but perhaps not ~thousands of memory allocations.
It works excellently in my small-ish game, and I also added some stats prints
to the report function to get some idea of when/why it starts to hit a
performance wall.
