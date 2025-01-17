**Contest link**

https://codeforces.com/blog/entry/137533

**Solution overview**

The first thing I noticed is that there is a bunch of bits in BoC serialisation which can just be omitted. For example, a entire block header with "number of roots" (which is always 1), etc, cell's level, subtree hashes stored in some special cells (they are only used for validation purposes during deserialisation).

Also, if edges always go backwards, one can calculate the minimal number of bits to store them and use exactly that number. However, interestingly, this didn't provide any boost after other optimisations described below.

Another thing came up when I was reading about LZ compression. Basically, it stores data as alternating pieces which are either "position in already decoded data, number of symbols to copy" or "string of symbols (called literal), number of symbols". In the description there was something along the lines "it's hard to find matching substrings so we use 64kB window and a hashmap mapping first 4 bytes to its last occurrence". Wait, what? What is the problem to just find the exact longest match? Having said that, I went ahead and implemented my own LZ compression based on suffix array and going through data with std::set. It gave a meaningful boost over the baseline solution at the time. Adjusting hyperparameters (e.g. minimal match length) also helped the score a bit.

Further, there was an intuition that similar data pieces are better to be closer to each other (e.g. a lot of bits were spent on encoding "match offset" and "match length"). It helped to realign the data and store d1's of all cells (i.e. number of refs and is_special), then all data sizes, then all refs and then all data.

After that, I threw per-byte huffman coding on top of the serialized and lz-ed data (huffman statistics were estimated from the sample test set and hardcoded into source code).

Also, I've built a "dictionary" of frequently appearing substrings in 25 sample tests and also hardcoded it into the source code. It further improved the score (the approach is to append this dictionary to the beginning of the data so lz can reference it naturally).

The above was the first milestone. After that, I got back to the documentation to actually learn something about the nature of data within cells. Shortly, there was another realisation (kind of obvious) — up to this point I've been working on bytes level instead of bits, while bitstrings inside cells are not aligned to 8 in any way!

So I switched to treating data as a bitstream and it actually improved the compression rate. But it turned out that suffix array construction was too slow now. Thankfully, there was a different approach: build suffix automaton of the reversed string and look at the suffix links. They form the suffix tree of the forward string. The average leaf depth was about 36 on sample data, so one can just take any suffix and iterate over its parent in this (implicit) suffix tree. I find this idea very cool, as it is not only ~20x faster and simpler in my implementation, but also allows to naturally skip all matches with a length less than a given threshold.

Another helpful idea was store lz match starting positions not as "offset to the current position", but as "offset to the current cell id" + "offset within a cell we have a match with". The intuition is that certain parts of cells should be matched more frequently than others.

At this point, there was not a lot of time left, so I did the following: calculated histograms of lz tokens ("literal length", "match length", "cell id offset of a match", "offset within the matching cell") over the sample data, hardcoded them to the source code and then used huffman coding during compression. Interestingly, a constant dictionary of frequently appearing bitstrings didn't help this time.

**Result**

1000 USDT and 20th place.
