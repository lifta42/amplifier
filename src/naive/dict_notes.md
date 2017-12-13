# Some Information About the Implementation of naive-dict. {#dict_notes}

There are two underlying strategy of getting/setting key-value pairs in
naive-dict (as `dict` in following). When the scale of a dictionary is small,
the data are stored linearly. Because there's no way to compare keys, so they
are left unsorted. If the dictionary grows bigger enough, the type will be
changed to hash. The hash method of key type will be used and open addressing
scheme will be applied on collisions. There's no way reverse to convert, because
`dict` does not support deletion.

At the beginning of `dict.c`, there are two constant definitions:

```C
static const int dict_initial      = 16;
static const int dict_hash_initial = 32;
```

The first one indicates the initial size set for a dictionary which is created
with [dict_create](@ref dict_create), and the second one indicates the cut-off
size of linear and hash dictionary.

The linear type dictionary is pretty much slower than the hash type one, but
a hash type dictionary takes up more space, since they are never full filed;
if a key takes too many times to look up and is not found eventually during an
invocation of [dict_put](@ref dict_put), the dictionary will be extended before
inserting the new key-value pair. The behavior is controlled by the following
constant:

```C
static const int dict_hash_longest = 4;
```

Which means the limitation of tolerance of times of looking up one key is 4 by
default. Notice that a dictionary will never be extended during a reading by
[dict_get](@ref dict_get), no matter how many times it takes to find the key and
whether the key is found or not, because getter function accepts a constant
dictionary as argument and cannot modify it.

The constant `dict_hash_longest` should never exceed `dict_hash_initial`, or
unexpected and undetermined consequence may be caused.

The three constant described above are chosen almost randomly. The `dict` module
was first implemented to complete the base of `dfa` module located in `auto`,
which represents DFA and its instance. A DFA is usually be created as the spirit
of a regex string, and such DFA's states would have tens of connections to other
states, and rarely up to 100, because there are not so much ASCII characters to
act as symbols. So the dictionaries of states in DFA probably have similar
numbers of keys. This is the reason of choosing such numbers as default of
`dict`.
