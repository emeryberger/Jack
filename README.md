Jack
----

Copyright (C) 2015-2016 Emery Berger

Jack is a new variant of the DieHard/DieHarder/Archipelago family of
memory managers, all of which aim to increase resilience to bugs
and/or attacks. They make certain kinds of errors impossible
(double-frees, accidental metadata corruption) or probabilistically
unlikely (dangling pointer / use-after-free errors, and buffer overflows).

Jack is a new point in the design space that aims to significantly
reduce space overhead and achieve O(1) allocation costs, while making
it difficult to predict the location of objects (since all objects are
randomly placed in memory).

UPDATE
------

Jack now incorporates a very different kind of allocator that never
recycles memory.  Instead, it leverages the enormous address spaces
available on 64-bit systems; for most applications, this is enough to
run for years without any issues.


