## The design of the little filesystem

A little fail-safe filesystem designed for embedded systems.

```
   | | |     .---._____
  .-----.   |          |
--|o    |---| littlefs |
--|     |---|          |
  '-----'   '----------'
   | | |
```

For a bit of backstory, the littlefs was developed with the goal of learning
more about filesystem design by tackling the relative unsolved problem of
managing a robust filesystem resilient to power loss on devices
with limited RAM and ROM.
`一点背景故事，LFS是为了一个目标而开发的：学习那些运行在极限RAM和ROM的设备上，
能解决掉电问题的强大文件系统。`


The embedded systems the littlefs is targeting are usually 32 bit
microcontrollers with around 32KB of RAM and 512KB of ROM. These are
often paired with SPI NOR flash chips with about 4MB of flash storage.
`LFS的目标嵌入式系统通常是32位系统，有32K RAM，512K ROM。通常还会配有一片4M的
SPI FLASH。`

granular
Flash itself is a very interesting piece of technology with quite a bit of
nuance. Unlike most other forms of storage, writing to flash requires two
operations: erasing and programming. The programming operation is relatively
cheap, and can be very granular. For NOR flash specifically, byte-level
programs are quite common. Erasing, however, requires an expensive operation
that forces the state of large blocks of memory to reset in a destructive
reaction that gives flash its name. The [Wikipedia entry](https://en.wikipedia.org/wiki/Flash_memory)
has more information if you are interested in how this works.
`FLASH本身是一个有细微差别的有趣技术。不像其他存储，写FLASH需要两步操作：
擦除和编程。编程操作相对便宜，并且可以非常精细，对于NOR FLASH， byte编程是
相当普遍的。但是擦除是一个代价很高的操作，就像FLASH意思，需要通过有破坏性的反应（FLASH）
强行将一大块memory的状态复位。`


This leaves us with an interesting set of limitations that can be simplified
to three strong requirements:
`这留给我们一系列有趣的限制，可以简化为3个强制要求：`


1. **Power-loss resilient** - This is the main goal of the littlefs and the
   focus of this project.
   `掉电保护 这是lfs的主要目标。`

   Embedded systems are usually designed without a shutdown routine and a
   notable lack of user interface for recovery, so filesystems targeting
   embedded systems must be prepared to lose power at any given time.
   `嵌入式系统通常没有设计关机流程，并缺乏用于恢复的用户界面。因此，
   用于嵌入式系统的文件系统必须准备好随时掉电。`
   
   Despite this state of things, there are very few embedded filesystems that
   handle power loss in a reasonable manner, and most can become corrupted if
   the user is unlucky enough.
   尽管有随时掉电的可能，很少嵌入式文件系统以合理的方式处理掉电，如果运气不好，
   掉电后文件系统就损坏了。

2. **Wear leveling** - Due to the destructive nature of flash, most flash
   chips have a limited number of erase cycles, usually in the order of around
   100,000 erases per block for NOR flash. Filesystems that don't take wear
   into account can easily burn through blocks used to store frequently updated
   metadata.
	磨损均衡，由于flash具有损耗性，大多数flash芯片有擦除次数限制，通常flash芯片每块的擦除次数为10万次左右。
	不考虑磨损的文件系统容易损坏那些用来频繁更新数据的flash块

   Consider the [FAT filesystem](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system),
   which stores a file allocation table (FAT) at a specific offset from the
   beginning of disk. Every block allocation will update this table, and after
   100,000 updates, the block will likely go bad, rendering the filesystem
   unusable even if there are many more erase cycles available on the storage
   as a whole.
   考虑到FAT文件系统在磁盘的特定位置（从起点偏移）存储一个FAT，每分配一个块都要更新这个FAT，当更新10万次之后，这个块近乎损坏，导致整个文件系统无法使用，即使存储器上还存在很多的擦除周期可用

3. **Bounded RAM/ROM** - Even with the design difficulties presented by the
   previous two limitations, we have already seen several flash filesystems
   developed on PCs that handle power loss just fine, such as the
   logging filesystems. However, these filesystems take advantage of the
   relatively cheap access to RAM, and use some rather... opportunistic...
   techniques, such as reconstructing the entire directory structure in RAM.
   These operations make perfect sense when the filesystem's only concern is
   erase cycles, but the idea is a bit silly on embedded systems.

	有限的RAM/ROM——即使有前面两个限制造成设计上的困难，我们已经看到几个在pc上开发的flash文件系统，可以很好地处理掉电，例如日志文件系统。然而，这些文件系统利用相对便宜的RAM，用一些投机取巧的技术，例如重构RAM里的整个目录结构。当文件系统唯一关心的是擦除时，这些操作就很完美，但是这个主意在嵌入式系统上有些愚蠢。

   To cater to embedded systems, the littlefs has the simple limitation of
   using only a bounded amount of RAM and ROM. That is, no matter what is
   written to the filesystem, and no matter how large the underlying storage
   is, the littlefs will always use the same amount of RAM and ROM. This
   presents a very unique challenge, and makes presumably simple operations,
   such as iterating through the directory tree, surprisingly difficult.
   为了契合对使用RAM和ROM有限制的嵌入式系统，LFS有一个简单的限制，这就是，不管向文件系统写什么，不管存储空间多大，LFS总是使用相同数量的RAM和ROM。这带来一个很独特的挑战，使一些简单的操作，例如遍历目录树，变得相当困难。
   

## Existing designs?
## 现有设计？

There are of course, many different existing filesystem. Here is a very rough
summary of the general ideas behind some of them.
当然，现有很多不同的文件系统。这里是一个对他们简单的总结。

Most of the existing filesystems fall into the one big category of filesystem
designed in the early days of spinny magnet disks. While there is a vast amount
of interesting technology and ideas in this area, the nature of spinny magnet
disks encourage properties, such as grouping writes near each other, that don't
make as much sense on recent storage types. For instance, on flash, write
locality is not important and can actually increase wear.
很多现有文件系统可以归类为为磁盘设计的文件系统。这有很多有趣的技术和点子。
一些由磁盘的特性引出的特性，例如组写操作彼此靠近，在当前的存储类型上不是很合适。
例如，在FLASH上，本地写（在同一个地方）并不总要，反而会增加磨损。

One of the most popular designs for flash filesystems is called the
[logging filesystem](https://en.wikipedia.org/wiki/Log-structured_file_system).
The flash filesystems [jffs](https://en.wikipedia.org/wiki/JFFS)
and [yaffs](https://en.wikipedia.org/wiki/YAFFS) are good examples. In a
logging filesystem, data is not stored in a data structure on disk, but instead
the changes to the files are stored on disk. This has several neat advantages,
such as the fact that the data is written in a cyclic log format and naturally
wear levels as a side effect. And, with a bit of error detection, the entire
filesystem can easily be designed to be resilient to power loss. The
journaling component of most modern day filesystems is actually a reduced
form of a logging filesystem. However, logging filesystems have a difficulty
scaling as the size of storage increases. And most filesystems compensate by
caching large parts of the filesystem in RAM, a strategy that is inappropriate
for embedded systems.
比较流行的FLASH文件系统叫做日志文件系统，例如JFFS，yaffs。
一个日志行文件系统，数据并没有保存在一个固定的磁盘数据结构中，
而是把文件的改动信息保存在磁盘上。这有几个优点,例如，由于数据以循环日志格式写入，
自然而然的就实现了磨损均衡。还有，当发现错误是，整个文件系统很容易恢复。
很多现在文件系统的日志组件实际上就是一个裁剪的日志文件系统。
但是，日志行文件系统随着存储内容的增加而显著增加。
很多日志行文件系统都可以将文件系统放到RAM中，这样做在嵌入式系统并不合适。

Another interesting filesystem design technique is that of [copy-on-write (COW)](https://en.wikipedia.org/wiki/Copy-on-write).
A good example of this is the [btrfs](https://en.wikipedia.org/wiki/Btrfs)
filesystem. COW filesystems can easily recover from corrupted blocks and have
natural protection against power loss. However, if they are not designed with
wear in mind, a COW filesystem could unintentionally wear down the root block
where the COW data structures are synchronized.
另外一种有趣的文件系统技术是COW。
btrfs是一个很好的例子。COW文件系统很容易从错误中恢复，并天生对掉电有保护作用。
但是，如果没有磨损考虑，COW文件系统很可能会将用于数据结构同步的ROOT块磨损。

## Metadata pairs
## 元数据对

The core piece of technology that provides the backbone for the littlefs is
the concept of metadata pairs. The key idea here is that any metadata that
needs to be updated atomically is stored on a pair of blocks tagged with
a revision count and checksum. Every update alternates between these two
pairs, so that at any time there is always a backup containing the previous
state of the metadata.
LFS主干的核心技术我们称之为元数据对。要点是，所有需要更新元数据都保存在
包含修订计数和校验和的一对数据块中。每次更新都在这两者之间交替进行对，
这样在任何时候总有一个包含前一个的备份元数据的状态。


Consider a small example where each metadata pair has a revision count,
a number as data, and the XOR of the block as a quick checksum. If
we update the data to a value of 9, and then to a value of 5, here is
what the pair of blocks may look like after each update:
先拿一个简单的元数据对做例子，这对数据对有一个修订计数，一个字节数据，
和一个简单的XOR校验值。
如果我们将这个数字改为9，再改为5，下面就是这对元数据对更新的过程：

```
  block 1   block 2        block 1   block 2        block 1   block 2
.---------.---------.    .---------.---------.    .---------.---------.
| rev: 1  | rev: 0  |    | rev: 1  | rev: 2  |    | rev: 3  | rev: 2  |
| data: 3 | data: 0 | -> | data: 3 | data: 9 | -> | data: 5 | data: 9 |
| xor: 2  | xor: 0  |    | xor: 2  | xor: 11 |    | xor: 6  | xor: 11 |
'---------'---------'    '---------'---------'    '---------'---------'
                 let data = 9             let data = 5
```

After each update, we can find the most up to date value of data by looking
at the revision count.
每次更新之后，我们可以通过修订计数查找到最新的值。

Now consider what the blocks may look like if we suddenly lose power while
changing the value of data to 5:
现在我们看看，如果在我们将数据修改为5时，系统突然掉电，元数据对是什么样子：

```
  block 1   block 2        block 1   block 2        block 1   block 2
.---------.---------.    .---------.---------.    .---------.---------.
| rev: 1  | rev: 0  |    | rev: 1  | rev: 2  |    | rev: 3  | rev: 2  |
| data: 3 | data: 0 | -> | data: 3 | data: 9 | -x | data: 3 | data: 9 |
| xor: 2  | xor: 0  |    | xor: 2  | xor: 11 |    | xor: 2  | xor: 11 |
'---------'---------'    '---------'---------'    '---------'---------'
                 let data = 9             let data = 5
                                          powerloss!!!
```

In this case, block 1 was partially written with a new revision count, but
the littlefs hadn't made it to updating the value of data. However, if we
check our checksum we notice that block 1 was corrupted. So we fall back to
block 2 and use the value 9.
在这种情况下，block1仅仅写了一个新的修订计数，并没有更新数据。
然而，如果我们检查校验值，可以发现这个块是损坏的。
因此我们回退到block2，使用数据9。

Using this concept, the littlefs is able to update metadata blocks atomically.
There are a few other tweaks, such as using a 32 bit CRC and using sequence
arithmetic to handle revision count overflow, but the basic concept
is the same. These metadata pairs define the backbone of the littlefs, and the
rest of the filesystem is built on top of these atomic updates.
通过这种方式，LFS更新元数据块就是一个最小的原子操作。
实际上有一点不一样，比如使用CRC-32作为校验值，使用序列算法处理修订计数溢出问题。
但基本原理是一样的。元数据对决定了LFS的基本实现方式，其他的文件系统操作都基于这个原子操作。


## Non-meta data
## 非元数据

Now, the metadata pairs do come with some drawbacks. Most notably, each pair
requires two blocks for each block of data. I'm sure users would be very
unhappy if their storage was suddenly cut in half! Instead of storing
everything in these metadata blocks, the littlefs uses a COW data structure
for files which is in turn pointed to by a metadata block. When
we update a file, we create copies of any blocks that are modified until
the metadata blocks are updated with the new copy. Once the metadata block
points to the new copy, we deallocate the old blocks that are no longer in use.
但是，元数据对也有一些缺点。最为显著的是，一对元数据对需要两块数据块。
容量减少一半，用户会非常不高兴。
LFS将文件数据保存在一个COW数据结构中，而不使用元数据对，当然，这个COW数据结构
由一个元数据块指定。
当我们更新文件，我们对要修改的块拷贝，然后更新元数据块，指向新的拷贝。
当元数据块更新完成，并指向新的数据块后，删除不在使用的旧数据块。

Here is what updating a one-block file may look like:
这是更新一块文件数据的情况：
```
  block 1   block 2        block 1   block 2        block 1   block 2
.---------.---------.    .---------.---------.    .---------.---------.
| rev: 1  | rev: 0  |    | rev: 1  | rev: 0  |    | rev: 1  | rev: 2  |
| file: 4 | file: 0 | -> | file: 4 | file: 0 | -> | file: 4 | file: 5 |
| xor: 5  | xor: 0  |    | xor: 5  | xor: 0  |    | xor: 5  | xor: 7  |
'---------'---------'    '---------'---------'    '---------'---------'
    |                        |                                  |
    v                        v                                  v
 block 4                  block 4    block 5       block 4    block 5
.--------.               .--------. .--------.    .--------. .--------.
| old    |               | old    | | new    |    | old    | | new    |
| data   |               | data   | | data   |    | data   | | data   |
|        |               |        | |        |    |        | |        |
'--------'               '--------' '--------'    '--------' '--------'
            update data in file        update metadata pair
```

It doesn't matter if we lose power while writing new data to block 5,
since the old data remains unmodified in block 4. This example also
highlights how the atomic updates of the metadata blocks provide a
synchronization barrier for the rest of the littlefs.
如果在我们将新数据写BLOCK5时系统掉电也没关系，因为block4还保留有未修改的数据。
这个例子还说明了原子更新元数据块如何和系统其它数据同步。

At this point, it may look like we are wasting an awfully large amount
of space on the metadata. Just looking at that example, we are using
three blocks to represent a file that fits comfortably in one! So instead
of giving each file a metadata pair, we actually store the metadata for
all files contained in a single directory in a single metadata block.
从这里看，我们好像浪费了大量的元数据空间。看刚刚的例子，我们使用3块数据
保存一个文件，但是这个文件其实在一个数据开就能放下。因此，我们不给文件分配元数据对，
而是将所有文件的元数据保存在单一目录，单一的元数据块中。

Now we could just leave files here, copying the entire file on write
provides the synchronization without the duplicated memory requirements
of the metadata blocks. However, we can do a bit better.
好的，现在我们可以将文件留在原处，拷贝整个文件
其实，我们还可以做得更好。

## CTZ skip-lists
## CTZ 跳跃表

There are many different data structures for representing the actual
files in filesystems. Of these, the littlefs uses a rather unique [COW](https://upload.wikimedia.org/wikipedia/commons/0/0c/Cow_female_black_white.jpg)
data structure that allows the filesystem to reuse unmodified parts of the
file without additional metadata pairs.
在一个文件系统中，文件需要各种不同的数据结构。
因此，LFS使用了一个独特的COW数据结构，这个数据结构允许文件系统利用文件未修改的
内容，而不需要增加元数据对。

First lets consider storing files in a simple linked-list. What happens when we
append a block? We have to change the last block in the linked-list to point
to this new block, which means we have to copy out the last block, and change
the second-to-last block, and then the third-to-last, and so on until we've
copied out the entire file.
首先考虑将文件保存在一个简单的链表。如何添加一个新块到链表呢？
需要将链表的最后一块指向新块，然后拷贝最后一块，倒数第二块，倒数第三块，直到
拷贝整个文件。


```
Exhibit A: A linked-list
.--------.  .--------.  .--------.  .--------.  .--------.  .--------.
| data 0 |->| data 1 |->| data 2 |->| data 4 |->| data 5 |->| data 6 |
|        |  |        |  |        |  |        |  |        |  |        |
|        |  |        |  |        |  |        |  |        |  |        |
'--------'  '--------'  '--------'  '--------'  '--------'  '--------'
```

To get around this, the littlefs, at its heart, stores files backwards. Each
block points to its predecessor, with the first block containing no pointers.
If you think about for a while, it starts to make a bit of sense. Appending
blocks just point to their predecessor and no other blocks need to be updated.
If we update a block in the middle, we will need to copy out the blocks that
follow, but can reuse the blocks before the modified block. Since most file
operations either reset the file each write or append to files, this design
avoids copying the file in the most common cases.
为了绕过这个问题，LFS，在核心设计上，文件使用反向存储方式。
每个块指向它的前一个块，第一块没有指向任何快。
如果你好好想想，这种设计有点意思：增加一个块，只需将这个块指向他的前一个块，
其他块都不需要更新。
如果我们更新中间的块，只需要拷贝后面的块，新块前面的块都可以不修改。
大部分文件操作，例如写入重置文件或添加文件数据，这种设计在大部分情况下
可以避免拷贝文件。

```
Exhibit B: A backwards linked-list
.--------.  .--------.  .--------.  .--------.  .--------.  .--------.
| data 0 |<-| data 1 |<-| data 2 |<-| data 4 |<-| data 5 |<-| data 6 |
|        |  |        |  |        |  |        |  |        |  |        |
|        |  |        |  |        |  |        |  |        |  |        |
'--------'  '--------'  '--------'  '--------'  '--------'  '--------'
```

However, a backwards linked-list does come with a rather glaring problem.
Iterating over a file _in order_ has a runtime cost of O(n^2). Gah! A quadratic
runtime to just _read_ a file? That's awful. Keep in mind reading files is
usually the most common filesystem operation.
然而，一个反向链表有一个很明显的问题。
遍历文件的时间是O(n^2)。哇！读一个文件多要二次方的运行时间？这是很可怕的。
记住，读文件是经常使用的文件系统操作。


To avoid this problem, the littlefs uses a multilayered linked-list. For
every nth block where n is divisible by 2^x, the block contains a pointer
to block n-2^x. So each block contains anywhere from 1 to log2(n) pointers
that skip to various sections of the preceding list. If you're familiar with
data-structures, you may have recognized that this is a type of deterministic
skip-list.
为了避免这个问题，LFS使用多层链表。对于n能被2^x整除的n个块。在块中包含指向块n-2^x
的指针。那么，每个块都包含任何位置从1到log2(n)块的指针，这样，就可以跳过每个快前面的
链表节点。如果你熟悉数据结构，你可能已经认识到，这既是一种确定的跳跃表。

The name comes from the use of the
[count trailing zeros (CTZ)](https://en.wikipedia.org/wiki/Count_trailing_zeros)
instruction, which allows us to calculate the power-of-two factors efficiently.
For a given block n, the block contains ctz(n)+1 pointers.
CTZ跳跃表

```
Exhibit C: A backwards CTZ skip-list
.--------.  .--------.  .--------.  .--------.  .--------.  .--------.
| data 0 |<-| data 1 |<-| data 2 |<-| data 3 |<-| data 4 |<-| data 5 |
|        |<-|        |--|        |<-|        |--|        |  |        |
|        |<-|        |--|        |--|        |--|        |  |        |
'--------'  '--------'  '--------'  '--------'  '--------'  '--------'
```

The additional pointers allow us to navigate the data-structure on disk
much more efficiently than in a singly linked-list.
增加的指针，我们可以比单链表更加有效的在磁盘上数据结构。

Taking exhibit C for example, here is the path from data block 5 to data
block 1. You can see how data block 3 was completely skipped:
这是从block5到block1的路径，你可以看出，block3 被我们跳过了。
```
.--------.  .--------.  .--------.  .--------.  .--------.  .--------.
| data 0 |  | data 1 |<-| data 2 |  | data 3 |  | data 4 |<-| data 5 |
|        |  |        |  |        |<-|        |--|        |  |        |
|        |  |        |  |        |  |        |  |        |  |        |
'--------'  '--------'  '--------'  '--------'  '--------'  '--------'
```

The path to data block 0 is even more quick, requiring only two jumps:
到block0的路径更快，只需要两步：
```
.--------.  .--------.  .--------.  .--------.  .--------.  .--------.
| data 0 |  | data 1 |  | data 2 |  | data 3 |  | data 4 |<-| data 5 |
|        |  |        |  |        |  |        |  |        |  |        |
|        |<-|        |--|        |--|        |--|        |  |        |
'--------'  '--------'  '--------'  '--------'  '--------'  '--------'
```

We can find the runtime complexity by looking at the path to any block from
the block containing the most pointers. Every step along the path divides
the search space for the block in half. This gives us a runtime of O(log n).
To get to the block with the most pointers, we can perform the same steps
backwards, which puts the runtime at O(2 log n) = O(log n). The interesting
part about this data structure is that this optimal path occurs naturally
if we greedily choose the pointer that covers the most distance without passing
our target block.
我们发现，运行时间复杂度，查找任何块，从包含多层指针的链表。每一步，都是寻找空间的一半。
这样，运行时间就是O(log n)，获取有多指针的块，我们可以执行向后执行相同的步骤。
这种数据结构有一个有趣的地方，最佳路径会自然出现，如果我们贪婪的选择指针覆盖更远
而不包含目标块。

So now we have a representation of files that can be appended trivially with
a runtime of O(1), and can be read with a worst case runtime of O(n log n).
Given that the the runtime is also divided by the amount of data we can store
in a block, this is pretty reasonable.
那么，我们可以保存一个文件，对这个文件追加数据的时间常数是O(1)，
读时间最差是O(n log n)。鉴于运行时间，只是我们保存的数据块总数的一半，这是非常合理的。

Unfortunately, the CTZ skip-list comes with a few questions that aren't
straightforward to answer. What is the overhead? How do we handle more
pointers than we can store in a block? How do we store the skip-list in
a directory entry?
很不幸的是，CTZ跳跃表也有一点问题，还是一些不好解决的问题。
什么是天花板（CTZ的顶部？）？如何处理多指针以便保存在一个块中？
如何在目录条目中保存跳跃表？

One way to find the overhead per block is to look at the data structure as
multiple layers of linked-lists. Each linked-list skips twice as many blocks
as the previous linked-list. Another way of looking at it is that each
linked-list uses half as much storage per block as the previous linked-list.
As we approach infinity, the number of pointers per block forms a geometric
series. Solving this geometric series gives us an average of only 2 pointers
per block.
寻找每个块的顶部的一种方法是，将数据结构看做链表的多层。每个链表都跳过其前链表的2倍。
另外一种方法是，每个链表使用前链表的一半。
当靠近极限，每个块的指针数是等比级数的形式。每个块只需要两个指针就能实现这个等比级数。

![overhead_per_block](https://latex.codecogs.com/svg.latex?%5Clim_%7Bn%5Cto%5Cinfty%7D%5Cfrac%7B1%7D%7Bn%7D%5Csum_%7Bi%3D0%7D%5E%7Bn%7D%5Cleft%28%5Ctext%7Bctz%7D%28i%29&plus;1%5Cright%29%20%3D%20%5Csum_%7Bi%3D0%7D%5Cfrac%7B1%7D%7B2%5Ei%7D%20%3D%202)

Finding the maximum number of pointers in a block is a bit more complicated,
but since our file size is limited by the integer width we use to store the
size, we can solve for it. Setting the overhead of the maximum pointers equal
to the block size we get the following equation. Note that a smaller block size
results in more pointers, and a larger word width results in larger pointers.
在块中寻找最大的指针有一点复杂。
但是文件最大也就是我们保存的宽度，可以通过这个来解决。

![maximum overhead](https://latex.codecogs.com/svg.latex?B%20%3D%20%5Cfrac%7Bw%7D%7B8%7D%5Cleft%5Clceil%5Clog_2%5Cleft%28%5Cfrac%7B2%5Ew%7D%7BB-2%5Cfrac%7Bw%7D%7B8%7D%7D%5Cright%29%5Cright%5Crceil)

where:  
B = block size in bytes  
w = word width in bits  

Solving the equation for B gives us the minimum block size for various word
widths:  
32 bit CTZ skip-list = minimum block size of 104 bytes  
64 bit CTZ skip-list = minimum block size of 448 bytes  
32位CTZ跳跃表，最小的block是104 bytes
64位的则是448 bytes

Since littlefs uses a 32 bit word size, we are limited to a minimum block
size of 104 bytes. This is a perfectly reasonable minimum block size, with most
block sizes starting around 512 bytes. So we can avoid additional logic to
avoid overflowing our block's capacity in the CTZ skip-list.
LFS用的是32bit字长，所以，最小块的极限是104字节。
这是一个完全合理的最小块，大部分情况下，FLASH的块是512字节。
因此，我们不需要为了避免CTZ跳跃表溢出而增加额外逻辑。

So, how do we store the skip-list in a directory entry? A naive approach would
be to store a pointer to the head of the skip-list, the length of the file
in bytes, the index of the head block in the skip-list, and the offset in the
head block in bytes. However this is a lot of information, and we can observe
that a file size maps to only one block index + offset pair. So it should be
sufficient to store only the pointer and file size.
那么，我们如何将跳跃表保存在目录条目？
一个天真的方法是，保存一个跳跃表的头指针，文件长度，跳跃表的第一块索引，头块的偏移。
然而，这是很多信息，我们可以发现，一个文件长度只对应一个块索引和偏移。
因此，只保存指针和文件大小就足够了。

But there is one problem, calculating the block index + offset pair from a
file size doesn't have an obvious implementation.
但是，这里有一个问题，从文件大小计算块索引和偏移对，没有明显的方法。

We can start by just writing down an equation. The first idea that comes to
mind is to just use a for loop to sum together blocks until we reach our
file size. We can write this equation as a summation:
我们从写下一个方程开始。
跳进脑子的第一个方法是，利用for循环统计所有的块，直到达到文件大小为止。

![summation1](https://latex.codecogs.com/svg.latex?N%20%3D%20%5Csum_i%5En%5Cleft%5BB-%5Cfrac%7Bw%7D%7B8%7D%5Cleft%28%5Ctext%7Bctz%7D%28i%29&plus;1%5Cright%29%5Cright%5D)

where:  
B = block size in bytes  
w = word width in bits  
n = block index in skip-list  
N = file size in bytes  

And this works quite well, but is not trivial to calculate. This equation
requires O(n) to compute, which brings the entire runtime of reading a file
to O(n^2 log n). Fortunately, the additional O(n) does not need to touch disk,
so it is not completely unreasonable. But if we could solve this equation into
a form that is easily computable, we can avoid a big slowdown.
这个方法能良好工作，但计算并不容易。这个方程需要O(n)的运行时间复杂度，因此会造成读
操作的时间复杂度是O(n^2 log n)。
幸运的是，新增的O(n)时间与磁盘无关，所以也并非完全不合理。
但是如果我们能找到一个容易计算的等式，就可以避免大幅减慢。

Unfortunately, the summation of the CTZ instruction presents a big challenge.
How would you even begin to reason about integrating a bitwise instruction?
不幸的是，CTZ统计是一个大挑战。

Fortunately, there is a powerful tool I've found useful in these situations:
The [On-Line Encyclopedia of Integer Sequences (OEIS)](https://oeis.org/).
幸运的是，我发现了一个强大的工具，OEIS(一个在线整数数列查询网站)

If we work out the first couple of values in our summation, we find that CTZ
maps to [A001511](https://oeis.org/A001511), and its partial summation maps
to [A005187](https://oeis.org/A005187), and surprisingly, both of these
sequences have relatively trivial equations! This leads us to a rather
unintuitive property:


![mindblown](https://latex.codecogs.com/svg.latex?%5Csum_i%5En%5Cleft%28%5Ctext%7Bctz%7D%28i%29&plus;1%5Cright%29%20%3D%202n-%5Ctext%7Bpopcount%7D%28n%29)

where:  
ctz(x) = the number of trailing bits that are 0 in x  
popcount(x) = the number of bits that are 1 in x  

It's a bit bewildering that these two seemingly unrelated bitwise instructions
are related by this property. But if we start to dissect this equation we can
see that it does hold. As n approaches infinity, we do end up with an average
overhead of 2 pointers as we find earlier. And popcount seems to handle the
error from this average as it accumulates in the CTZ skip-list.
这有点扑朔迷离，这两个看起来无关的位指令却与这个属性有关。
但是，如果我们开始解刨这个等式，我们看到，它是成立的。
当n接近极限时，我们最终得到一个2指针的平均开销，
这就像我们前面发现的一样。
而且，popcount看起来还处理了再CTZ跳跃表中累积的误差。

Now we can substitute into the original equation to get a trivial equation
for a file size:
现在我们可以替代进去原始的等式，得到一个精细的等式，用于计算文件尺寸。

![summation2](https://latex.codecogs.com/svg.latex?N%20%3D%20Bn%20-%20%5Cfrac%7Bw%7D%7B8%7D%5Cleft%282n-%5Ctext%7Bpopcount%7D%28n%29%5Cright%29)

Unfortunately, we're not quite done. The popcount function is non-injective,
so we can only find the file size from the block index, not the other way
around. However, we can solve for an n' block index that is greater than n
with an error bounded by the range of the popcount function. We can then
repeatedly substitute this n' into the original equation until the error
is smaller than the integer division. As it turns out, we only need to
perform this substitution once. Now we directly calculate our block index:
很不幸，我们还没完成。popcount函数不是一个内射函数。
因此我们只能通过BLOCK索引寻找文件尺寸，没有其他方法。


![formulaforn](https://latex.codecogs.com/svg.latex?n%20%3D%20%5Cleft%5Clfloor%5Cfrac%7BN-%5Cfrac%7Bw%7D%7B8%7D%5Cleft%28%5Ctext%7Bpopcount%7D%5Cleft%28%5Cfrac%7BN%7D%7BB-2%5Cfrac%7Bw%7D%7B8%7D%7D-1%5Cright%29&plus;2%5Cright%29%7D%7BB-2%5Cfrac%7Bw%7D%7B8%7D%7D%5Cright%5Crfloor)

Now that we have our block index n, we can just plug it back into the above
equation to find the offset. However, we do need to rearrange the equation
a bit to avoid integer overflow:
现在我们已经找到块索引n，我们将其查到上面的等式，找出偏移。
我们需要改编这个等式避免整型溢出：

![formulaforoff](https://latex.codecogs.com/svg.latex?%5Cmathit%7Boff%7D%20%3D%20N%20-%20%5Cleft%28B-2%5Cfrac%7Bw%7D%7B8%7D%5Cright%29n%20-%20%5Cfrac%7Bw%7D%7B8%7D%5Ctext%7Bpopcount%7D%28n%29)

The solution involves quite a bit of math, but computers are very good at math.
Now we can solve for both the block index and offset from the file size in O(1).
解决方案设计相当多的数学知识，但是，电脑对数学非常在行。
现在，我们已经从文件尺寸得到块索引和偏移，而且时间复杂度是O(1)。

Here is what it might look like to update a file stored with a CTZ skip-list:
这是更新一个用CTZ跳跃表保存的文件的过程：
```
                                      block 1   block 2
                                    .---------.---------.
                                    | rev: 1  | rev: 0  |
                                    | file: 6 | file: 0 |
                                    | size: 4 | size: 0 |
                                    | xor: 3  | xor: 0  |
                                    '---------'---------'
                                        |
                                        v
  block 3     block 4     block 5     block 6
.--------.  .--------.  .--------.  .--------.
| data 0 |<-| data 1 |<-| data 2 |<-| data 3 |
|        |<-|        |--|        |  |        |
|        |  |        |  |        |  |        |
'--------'  '--------'  '--------'  '--------'

|  update data in file
v

                                      block 1   block 2
                                    .---------.---------.
                                    | rev: 1  | rev: 0  |
                                    | file: 6 | file: 0 |
                                    | size: 4 | size: 0 |
                                    | xor: 3  | xor: 0  |
                                    '---------'---------'
                                        |
                                        v
  block 3     block 4     block 5     block 6
.--------.  .--------.  .--------.  .--------.
| data 0 |<-| data 1 |<-| old    |<-| old    |
|        |<-|        |--| data 2 |  | data 3 |
|        |  |        |  |        |  |        |
'--------'  '--------'  '--------'  '--------'
     ^ ^           ^
     | |           |      block 7     block 8     block 9    block 10
     | |           |    .--------.  .--------.  .--------.  .--------.
     | |           '----| new    |<-| new    |<-| new    |<-| new    |
     | '----------------| data 2 |<-| data 3 |--| data 4 |  | data 5 |
     '------------------|        |--|        |--|        |  |        |
                        '--------'  '--------'  '--------'  '--------'

|  update metadata pair
v

                                                   block 1   block 2
                                                 .---------.---------.
                                                 | rev: 1  | rev: 2  |
                                                 | file: 6 | file: 10|
                                                 | size: 4 | size: 6 |
                                                 | xor: 3  | xor: 14 |
                                                 '---------'---------'
                                                                |
                                                                |
  block 3     block 4     block 5     block 6                   |
.--------.  .--------.  .--------.  .--------.                  |
| data 0 |<-| data 1 |<-| old    |<-| old    |                  |
|        |<-|        |--| data 2 |  | data 3 |                  |
|        |  |        |  |        |  |        |                  |
'--------'  '--------'  '--------'  '--------'                  |
     ^ ^           ^                                            v
     | |           |      block 7     block 8     block 9    block 10
     | |           |    .--------.  .--------.  .--------.  .--------.
     | |           '----| new    |<-| new    |<-| new    |<-| new    |
     | '----------------| data 2 |<-| data 3 |--| data 4 |  | data 5 |
     '------------------|        |--|        |--|        |  |        |
                        '--------'  '--------'  '--------'  '--------'
```

## Block allocation
## 块分配

So those two ideas provide the grounds for the filesystem. The metadata pairs
give us directories, and the CTZ skip-lists give us files. But this leaves
one big [elephant](https://upload.wikimedia.org/wikipedia/commons/3/37/African_Bush_Elephant.jpg)
of a question. How do we get those blocks in the first place?
所以这两个想法为文件系统提供了基础。
元数据对给了我们目录。CTZ跳跃链表给了我们文件。
但是，留下了一个大问题：如何获取这些块？

One common strategy is to store unallocated blocks in a big free list, and
initially the littlefs was designed with this in mind. By storing a reference
to the free list in every single metadata pair, additions to the free list
could be updated atomically at the same time the replacement blocks were
stored in the metadata pair. During boot, every metadata pair had to be
scanned to find the most recent free list, but once the list was found the
state of all free blocks becomes known.
一个共同的策略就是讲为分配的块保存在一个大的空闲块列表中，LFS开始的版本也是这样设计的。


However, this approach had several issues:
可惜，这方法有几个问题：
- There was a lot of nuanced logic for adding blocks to the free list without
  modifying the blocks, since the blocks remain active until the metadata is
  updated.
- The free list had to support both additions and removals in FIFO order while
  minimizing block erases.
- The free list had to handle the case where the file system completely ran
  out of blocks and may no longer be able to add blocks to the free list.
- If we used a revision count to track the most recently updated free list,
  metadata blocks that were left unmodified were ticking time bombs that would
  cause the system to go haywire if the revision count overflowed.
- Every single metadata block wasted space to store these free list references.

Actually, to simplify, this approach had one massive glaring issue: complexity.
其实，简单的说，这个方法有一个巨大的明显问题：复杂度。

> Complexity leads to fallibility.  
> Fallibility leads to unmaintainability.  
> Unmaintainability leads to suffering.  
复杂度导致易错。
易错导致可维护性差。
可维护性差导致痛苦。

Or at least, complexity leads to increased code size, which is a problem
for embedded systems.
至少，复杂度增加代码量，这在嵌入式系统时一个问题。

In the end, the littlefs adopted more of a "drop it on the floor" strategy.
That is, the littlefs doesn't actually store information about which blocks
are free on the storage. The littlefs already stores which files _are_ in
use, so to find a free block, the littlefs just takes all of the blocks that
exist and subtract the blocks that are in use.
最后，LFS采用更多的“放在地板上”策略。
也就是，LFS其实没有在存储上保存空闲块的信息。LFS已经保存了文件使用块的信息，
要找空闲块，只需要找出所有块，减去已经使用块。

Of course, it's not quite that simple. Most filesystems that adopt this "drop
it on the floor" strategy either rely on some properties inherent to the
filesystem, such as the cyclic-buffer structure of logging filesystems,
or use a bitmap or table stored in RAM to track free blocks, which scales
with the size of storage and is problematic when you have limited RAM. You
could iterate through every single block in storage and check it against
every single block in the filesystem on every single allocation, but that
would have an abhorrent runtime.
当然，实际上没这么简单。很多才采用"drop it on the floor" 策略的文件系统，
要不就是依赖一些文件系统固有的特性，例如循环缓冲区结构的日志文件系统，
或者是使用存储在RAM的位图或者表格追踪空闲块，RAM的使用随着文件系统容量变大，
如果你的RAM没有多少，这会成为一个问题。
你可以遍历存储中的单个模块，并且在每次分配块时比较，如此将造成恐怖的运行时间。

So the littlefs compromises. It doesn't store a bitmap the size of the storage,
but it does store a little bit-vector that contains a fixed set lookahead
for block allocations. During a block allocation, the lookahead vector is
checked for any free blocks. If there are none, the lookahead region jumps
forward and the entire filesystem is scanned for free blocks.
没办法，LFS只好妥协了。
LFS没有保存存储的位图，但保存了一个小的位向量，这个向量包含一个固定的块分配预测。
在块分配时，通过检测预测向量查找空闲块。如果没找到空闲块，预测区域向前跳跃，
并扫描整个文件系统查找空闲块。

Here's what it might look like to allocate 4 blocks on a decently busy
filesystem with a 32bit lookahead and a total of
128 blocks (512Kbytes of storage if blocks are 4Kbyte):
这是在一个已经快满的文件系统分配4块的例子，预测头是32位。
总共有128个块，

```
boot...         lookahead:
                fs blocks: fffff9fffffffffeffffffffffff0000
scanning...     lookahead: fffff9ff
                fs blocks: fffff9fffffffffeffffffffffff0000
alloc = 21      lookahead: fffffdff
                fs blocks: fffffdfffffffffeffffffffffff0000
alloc = 22      lookahead: ffffffff
                fs blocks: fffffffffffffffeffffffffffff0000
scanning...     lookahead:         fffffffe
                fs blocks: fffffffffffffffeffffffffffff0000
alloc = 63      lookahead:         ffffffff
                fs blocks: ffffffffffffffffffffffffffff0000
scanning...     lookahead:         ffffffff
                fs blocks: ffffffffffffffffffffffffffff0000
scanning...     lookahead:                 ffffffff
                fs blocks: ffffffffffffffffffffffffffff0000
scanning...     lookahead:                         ffff0000
                fs blocks: ffffffffffffffffffffffffffff0000
alloc = 112     lookahead:                         ffff8000
                fs blocks: ffffffffffffffffffffffffffff8000
```

While this lookahead approach still has an asymptotic runtime of O(n^2) to
scan all of storage, the lookahead reduces the practical runtime to a
reasonable amount. Bit-vectors are surprisingly compact, given only 16 bytes,
the lookahead could track 128 blocks. For a 4Mbyte flash chip with 4Kbyte
blocks, the littlefs would only need 8 passes to scan the entire storage.
虽然这种方法扫描整个存储仍然是O(n^2)时间复杂度，但减少实际运行时间到一个合理量。
位向量非常紧凑，只需要16字节，就可以追踪128块。
一个4M空间，4K一个块，LFS只需要8次，就可以扫描整个存储。

The real benefit of this approach is just how much it simplified the design
of the littlefs. Deallocating blocks is as simple as simply forgetting they
exist, and there is absolutely no concern of bugs in the deallocation code
causing difficult to detect memory leaks.
这种方法的真正好处是大大简化了LFS的设计。
回收块就像忘记他们一样简单，而且根本不用担心回收快代码中的错误，造成很难察觉的内存泄漏。

## Directories目录

Now we just need directories to store our files. Since we already have
metadata blocks that store information about files, lets just use these
metadata blocks as the directories. Maybe turn the directories into linked
lists of metadata blocks so it isn't limited by the number of files that fit
in a single block. Add entries that represent other nested directories.
Drop "." and ".." entries, cause who needs them. Dust off our hands and
we now have a directory tree.
我们需要目录保存我们的文件。
现在，我们已经有元数据块保存文件信息，让我们也用元数据块保存目录。
把目录转换为元数据块链表，这样就不受一个块能保存的文件数限制。
添加其他网状目录的条目。
不要用 "."和".."条目，有人需要他们。
甩甩手，放松下，我们现在有目录树了。


```
            .--------.
            |root dir|
            | pair 0 |
            |        |
            '--------'
            .-'    '-------------------------.
           v                                  v
      .--------.        .--------.        .--------.
      | dir A  |------->| dir A  |        | dir B  |
      | pair 0 |        | pair 1 |        | pair 0 |
      |        |        |        |        |        |
      '--------'        '--------'        '--------'
      .-'    '-.            |             .-'    '-.
     v          v           v            v          v
.--------.  .--------.  .--------.  .--------.  .--------.
| file C |  | file D |  | file E |  | file F |  | file G |
|        |  |        |  |        |  |        |  |        |
|        |  |        |  |        |  |        |  |        |
'--------'  '--------'  '--------'  '--------'  '--------'
```

Unfortunately it turns out it's not that simple. See, iterating over a
directory tree isn't actually all that easy, especially when you're trying
to fit in a bounded amount of RAM, which rules out any recursive solution.
And since our block allocator involves iterating over the entire filesystem
tree, possibly multiple times in a single allocation, iteration needs to be
efficient.
不幸的是，事实并没有怎么简单。
请看，遍历一个目录树其实没那么容易，特别是你想在一个有限的RAM空间上实现，
因为任何递归方法都不可用。
我们的块分配器设计遍历整个文件系统数，而且一次分配可能遍历多次，遍历需要更有效。

So, as a solution, the littlefs adopted a sort of threaded tree. Each
directory not only contains pointers to all of its children, but also a
pointer to the next directory. These pointers create a linked-list that
is threaded through all of the directories in the filesystem. Since we
only use this linked list to check for existence, the order doesn't actually
matter. As an added plus, we can repurpose the pointer for the individual
directory linked-lists and avoid using any additional space.
额，方案就是，LFS使用了一堆线程树。
一个目录，不仅仅包含指向他所有孩子的指针，还包含指向下一个目录的指针。
这些指针形成一个链表，串起文件系统的所有目录。
因为我们只是用这个链表检查目录是否存在，顺序无关紧要。
另外，我们可以为个别目录链表重新规划指针，避免使用额外的空间。

```
            .--------.
            |root dir|-.
            | pair 0 | |
   .--------|        |-'
   |        '--------'
   |        .-'    '-------------------------.
   |       v                                  v
   |  .--------.        .--------.        .--------.
   '->| dir A  |------->| dir A  |------->| dir B  |
      | pair 0 |        | pair 1 |        | pair 0 |
      |        |        |        |        |        |
      '--------'        '--------'        '--------'
      .-'    '-.            |             .-'    '-.
     v          v           v            v          v
.--------.  .--------.  .--------.  .--------.  .--------.
| file C |  | file D |  | file E |  | file F |  | file G |
|        |  |        |  |        |  |        |  |        |
|        |  |        |  |        |  |        |  |        |
'--------'  '--------'  '--------'  '--------'  '--------'
```

This threaded tree approach does come with a few tradeoffs. Now, anytime we
want to manipulate the directory tree, we find ourselves having to update two
pointers instead of one. For anyone familiar with creating atomic data
structures this should set off a whole bunch of red flags.
线程树方法伴随而来一些新的权衡。
现在，我们发现，只要我们操作目录树，就需要更新2个指针，而原来只要更新一个。
对于熟悉创建原子数据结构的人来说，这应该引发一大堆红旗。

But unlike the data structure guys, we can update a whole block atomically! So
as long as we're really careful (and cheat a little bit), we can still
manipulate the directory tree in a way that is resilient to power loss.
但我们不像那些数据结构的家伙，我们可以原子更新整个块。
因此，只要我们小心（稍微作弊），我们可以操作整个目录树，而不用担心突然掉电。

Consider how we might add a new directory. Since both pointers that reference
it can come from the same directory, we only need a single atomic update to
finagle the directory into the filesystem:
思考一下我们是如何添加新目录的。
因为两个指针可能指向同一个目录。我们只需要一个原子更新来将目录导入文件系统：

```
   .--------.
   |root dir|-.
   | pair 0 | |
.--|        |-'
|  '--------'
|      |
|      v
|  .--------.
'->| dir A  |
   | pair 0 |
   |        |
   '--------'

|  create the new directory block
v

               .--------.
               |root dir|-.
               | pair 0 | |
            .--|        |-'
            |  '--------'
            |      |
            |      v
            |  .--------.
.--------.  '->| dir A  |
| dir B  |---->| pair 0 |
| pair 0 |     |        |
|        |     '--------'
'--------'

|  update root to point to directory B
v

         .--------.
         |root dir|-.
         | pair 0 | |
.--------|        |-'
|        '--------'
|        .-'    '-.
|       v          v
|  .--------.  .--------.
'->| dir B  |->| dir A  |
   | pair 0 |  | pair 0 |
   |        |  |        |
   '--------'  '--------'
```

Note that even though directory B was added after directory A, we insert
directory B before directory A in the linked-list because it is convenient.
请注意，尽管B目录在A目录之后添加，我们却将B目录放在链表中A目录之前，
因为这样比较方便。

Now how about removal:
```
         .--------.        .--------.
         |root dir|------->|root dir|-.
         | pair 0 |        | pair 1 | |
.--------|        |--------|        |-'
|        '--------'        '--------'
|        .-'    '-.            |
|       v          v           v
|  .--------.  .--------.  .--------.
'->| dir A  |->| dir B  |->| dir C  |
   | pair 0 |  | pair 0 |  | pair 0 |
   |        |  |        |  |        |
   '--------'  '--------'  '--------'

|  update root to no longer contain directory B
v

   .--------.              .--------.
   |root dir|------------->|root dir|-.
   | pair 0 |              | pair 1 | |
.--|        |--------------|        |-'
|  '--------'              '--------'
|      |                       |
|      v                       v
|  .--------.  .--------.  .--------.
'->| dir A  |->| dir B  |->| dir C  |
   | pair 0 |  | pair 0 |  | pair 0 |
   |        |  |        |  |        |
   '--------'  '--------'  '--------'

|  remove directory B from the linked-list
v

   .--------.  .--------.
   |root dir|->|root dir|-.
   | pair 0 |  | pair 1 | |
.--|        |--|        |-'
|  '--------'  '--------'
|      |           |
|      v           v
|  .--------.  .--------.
'->| dir A  |->| dir C  |
   | pair 0 |  | pair 0 |
   |        |  |        |
   '--------'  '--------'
```

Wait, wait, wait, that's not atomic at all! If power is lost after removing
directory B from the root, directory B is still in the linked-list. We've
just created a memory leak!
等等，这根本不是原子操作。
如果在将B目录从根目录移除时发生掉电，目录B仍然在链表中。
我们刚刚造成内存泄漏了。

And to be honest, I don't have a clever solution for this case. As a
side-effect of using multiple pointers in the threaded tree, the littlefs
can end up with orphan blocks that have no parents and should have been
removed.
老实说，我太笨，不知道如何解决这种情况。
使用多指针的线程树的副作用，LFS可能会存在应该被移除的孤儿块。

To keep these orphan blocks from becoming a problem, the littlefs has a
deorphan step that simply iterates through every directory in the linked-list
and checks it against every directory entry in the filesystem to see if it
has a parent. The deorphan step occurs on the first block allocation after
boot, so orphans should never cause the littlefs to run out of storage
prematurely. Note that the deorphan step never needs to run in a read-only
filesystem.
为了防止这些孤儿块成为问题，LFS有一个回收孤儿块的步骤。
只需简单遍历链表中的每一个目录，并和文件系统中的所有目录比较，看他到底有没有父母。
LFS在启动后第一次块分配是进行孤儿块回收。
因此，孤儿块应该不会造成LFS耗尽存储空间。
注意，在只读文件系统中，不需要孤儿块回收。

## The move problem

Now we have a real problem. How do we move things between directories while
remaining power resilient? Even looking at the problem from a high level,
it seems impossible. We can update directory blocks atomically, but atomically
updating two independent directory blocks is not an atomic operation.
现在有一个真正的问题需要考虑，在两个目录之间移动数据时，如何防止掉电损坏？
尽管在宏观上看来，这不可能发生。
我们可以原子更新目录块，但是原子更新两个独立的目录块并不是原子操作。

Here's the steps the filesystem may go through to move a directory:
```
         .--------.
         |root dir|-.
         | pair 0 | |
.--------|        |-'
|        '--------'
|        .-'    '-.
|       v          v
|  .--------.  .--------.
'->| dir A  |->| dir B  |
   | pair 0 |  | pair 0 |
   |        |  |        |
   '--------'  '--------'

|  update directory B to point to directory A
v

         .--------.
         |root dir|-.
         | pair 0 | |
.--------|        |-'
|        '--------'
|    .-----'    '-.
|    |             v
|    |           .--------.
|    |        .->| dir B  |
|    |        |  | pair 0 |
|    |        |  |        |
|    |        |  '--------'
|    |     .-------'
|    v    v   |
|  .--------. |
'->| dir A  |-'
   | pair 0 |
   |        |
   '--------'

|  update root to no longer contain directory A
v
     .--------.
     |root dir|-.
     | pair 0 | |
.----|        |-'
|    '--------'
|        |
|        v
|    .--------.
| .->| dir B  |
| |  | pair 0 |
| '--|        |-.
|    '--------' |
|        |      |
|        v      |
|    .--------. |
'--->| dir A  |-'
     | pair 0 |
     |        |
     '--------'
```

We can leave any orphans up to the deorphan step to collect, but that doesn't
help the case where dir A has both dir B and the root dir as parents if we
lose power inconveniently.
我们可以将孤儿块留到孤儿回收是收集。
但是对这种情况无效：A有两个父节点。

Initially, you might think this is fine. Dir A _might_ end up with two parents,
but the filesystem will still work as intended. But then this raises the
question of what do we do when the dir A wears out? For other directory blocks
we can update the parent pointer, but for a dir with two parents we would need
work out how to update both parents. And the check for multiple parents would
need to be carried out for every directory, even if the directory has never
been moved.
最初，你可能认为这没有问题。DIR A有两个父节点，但是文件系统仍然能按照预期工作。
但是这会引起一个问题，当目录A磨损了，怎么办？对于其他节点，只要更新父节点指针，
但是对于有两个父节点的，如何更新？

It also presents a bad user-experience, since the condition of ending up with
two parents is rare, it's unlikely user-level code will be prepared. Just think
about how a user would recover from a multi-parented directory. They can't just
remove one directory, since remove would report the directory as "not empty".
这也造成一个不好的用户体验，因为以两个父节点结束的情况很少，用户代码不太可能预料到。试想一下，用户怎么从多个父结点目录中恢复，他们不能简单的删除一个目录，因为这个目录“非空”

Other atomic filesystems simple COW the entire directory tree. But this
introduces a significant bit of complexity, which leads to code size, along
with a surprisingly expensive runtime cost during what most users assume is
a single pointer update.
其他原子文件系统简单的COW整个目录树。但是这引入一个相当复杂的问题，这个问题导致很多用户以为更新一个指针，却花费惊人的运行时间


Another option is to update the directory block we're moving from to point
to the destination with a sort of predicate that we have moved if the
destination exists. Unfortunately, the omnipresent concern of wear could
cause any of these directory entries to change blocks, and changing the
entry size before a move introduces complications if it spills out of
the current directory block.
另一个选择是更新（我们从point搬到目的地的）目录块，前提是这个目的地存在。不幸的是，磨损均衡会造成这些目录进入变化块，并且，如果在当前的目录块溢出，会（在一个移动引入多bugs之前）改变入口大小


So how do we go about moving a directory atomically?
那么我们怎么做到原子移动一个目录？

We rely on the improbableness of power loss.
我们依赖（）掉电

Power loss during a move is certainly possible, but it's actually relatively
rare. Unless a device is writing to a filesystem constantly, it's unlikely that
a power loss will occur during filesystem activity. We still need to handle
the condition, but runtime during a power loss takes a back seat to the runtime
during normal operations.
在移动期间掉电是很可能的，但是，确实很稀少。除非不断地向文件系统写数据，不然在文件系统工作期间掉电很少见。我们依然需要处理这种情况，但是，运行时间（在掉电期间相对于正常操作时）变得次要，

So what littlefs does is inelegantly simple. When littlefs moves a file, it
marks the file as "moving". This is stored as a single bit in the directory
entry and doesn't take up much space. Then littlefs moves the directory,
finishing with the complete remove of the "moving" directory entry.
所以，LFS做的很简单。当LFS移动一个文件，就标记为“正在移动”。这个存储在入口目录的bit上，且不占用多少空间。当整个“moving”目录删除结束时，LFS移动目录完成


```
         .--------.
         |root dir|-.
         | pair 0 | |
.--------|        |-'
|        '--------'
|        .-'    '-.
|       v          v
|  .--------.  .--------.
'->| dir A  |->| dir B  |
   | pair 0 |  | pair 0 |
   |        |  |        |
   '--------'  '--------'

|  update root directory to mark directory A as moving
v

        .----------.
        |root dir  |-.
        | pair 0   | |
.-------| moving A!|-'
|       '----------'
|        .-'    '-.
|       v          v
|  .--------.  .--------.
'->| dir A  |->| dir B  |
   | pair 0 |  | pair 0 |
   |        |  |        |
   '--------'  '--------'

|  update directory B to point to directory A
v

        .----------.
        |root dir  |-.
        | pair 0   | |
.-------| moving A!|-'
|       '----------'
|    .-----'    '-.
|    |             v
|    |           .--------.
|    |        .->| dir B  |
|    |        |  | pair 0 |
|    |        |  |        |
|    |        |  '--------'
|    |     .-------'
|    v    v   |
|  .--------. |
'->| dir A  |-'
   | pair 0 |
   |        |
   '--------'

|  update root to no longer contain directory A
v
     .--------.
     |root dir|-.
     | pair 0 | |
.----|        |-'
|    '--------'
|        |
|        v
|    .--------.
| .->| dir B  |
| |  | pair 0 |
| '--|        |-.
|    '--------' |
|        |      |
|        v      |
|    .--------. |
'--->| dir A  |-'
     | pair 0 |
     |        |
     '--------'
```

Now, if we run into a directory entry that has been marked as "moved", one
of two things is possible. Either the directory entry exists elsewhere in the
filesystem, or it doesn't. This is a O(n) operation, but only occurs in the
unlikely case we lost power during a move.
现在，如果我们进入一个被标记为“moved”的目录，两种情况中的一个有可能发生。不管目录入口存在文件系统的任何位置，还是不存在。存在一个O(n)操作，但是，仅仅在非常不幸地的情况下发生（当我们移动时掉电）

And we can easily fix the "moved" directory entry. Since we're already scanning
the filesystem during the deorphan step, we can also check for moved entries.
If we find one, we either remove the "moved" marking or remove the whole entry
if it exists elsewhere in the filesystem.


## Wear awareness 磨损意识

So now that we have all of the pieces of a filesystem, we can look at a more
subtle attribute of embedded storage: The wear down of flash blocks.
至此，我们得到一个完整的文件系统。
我们可以看一下嵌入式存储的一个更微妙的属性：FLASH块磨损。

The first concern for the littlefs, is that perfectly valid blocks can suddenly
become unusable. As a nice side-effect of using a COW data-structure for files,
we can simply move on to a different block when a file write fails. All
modifications to files are performed in copies, so we will only replace the
old file when we are sure none of the new file has errors. Directories, on
the other hand, need a different strategy.
LFS关心的一个问题就是，完美避免一个数据块突然坏掉。
由于文件使用COW数据结构，有一个好的副作用，当一个文件写失败时，可以简单的移动到另外一个块。
所有文件操作都有备份。
目录，需要一个不同的策略。

The solution to directory corruption in the littlefs relies on the redundant
nature of the metadata pairs. If an error is detected during a write to one
of the metadata pairs, we seek out a new block to take its place. Once we find
a block without errors, we iterate through the directory tree, updating any
references to the corrupted metadata pair to point to the new metadata block.
Just like when we remove directories, we can lose power during this operation
and end up with a desynchronized metadata pair in our filesystem. And just like
when we remove directories, we leave the possibility of a desynchronized
metadata pair up to the deorphan step to clean up.
LFS解决目录损坏的方法依赖其具有元数据对的特性。如果向元数据对中的一个写数据期间检测到错误，我们找到一个新的块去代替。一旦我们找到一个没有错误的块，我们遍历目录树，更新（任何对坏元数据的）引用指向新的元数据块。就像当我们删除目录期间可以掉电，会在文件系统中添加一个不同步的元数据对。也像当我们删除目录，我们留一个不同步的（up to the 非单独）元数据对去清除

Here's what encountering a directory error may look like with all of
the directories and directory pointers fully expanded:
```
下图是在所有目录和目录指针全展开的情况下，一个目录错误可能出现的bug

         root dir
         block 1   block 2
       .---------.---------.
       | rev: 1  | rev: 0  |--.
       |         |         |-.|
.------|         |         |-|'
|.-----|         |         |-'
||     '---------'---------'
||       |||||'--------------------------------------------------.
||       ||||'-----------------------------------------.         |
||       |||'-----------------------------.            |         |
||       ||'--------------------.         |            |         |
||       |'-------.             |         |            |         |
||       v         v            v         v            v         v
||    dir A                  dir B                  dir C
||    block 3   block 4      block 5   block 6      block 7   block 8
||  .---------.---------.  .---------.---------.  .---------.---------.
|'->| rev: 1  | rev: 0  |->| rev: 1  | rev: 0  |->| rev: 1  | rev: 0  |
'-->|         |         |->|         |         |->|         |         |
    |         |         |  |         |         |  |
    |         |         |  |         |         |  |         |         |
    '---------'---------'  '---------'---------'  '---------'---------'

|  update directory B
v

         root dir
         block 1   block 2
       .---------.---------.
       | rev: 1  | rev: 0  |--.
       |         |         |-.|
.------|         |         |-|'
|.-----|         |         |-'
||     '---------'---------'
||       |||||'--------------------------------------------------.
||       ||||'-----------------------------------------.         |
||       |||'-----------------------------.            |         |
||       ||'--------------------.         |            |         |
||       |'-------.             |         |            |         |
||       v         v            v         v            v         v
||    dir A                  dir B                  dir C
||    block 3   block 4      block 5   block 6      block 7   block 8
||  .---------.---------.  .---------.---------.  .---------.---------.
|'->| rev: 1  | rev: 0  |->| rev: 1  | rev: 2  |->| rev: 1  | rev: 0  |
'-->|         |         |->|         | corrupt!|->|         |         |
    |         |         |  |         | corrupt!|  |         |         |
    |         |         |  |         | corrupt!|  |         |         |
    '---------'---------'  '---------'---------'  '---------'---------'

|  oh no! corruption detected
v  allocate a replacement block

         root dir
         block 1   block 2
       .---------.---------.
       | rev: 1  | rev: 0  |--.
       |         |         |-.|
.------|         |         |-|'
|.-----|         |         |-'
||     '---------'---------'
||       |||||'----------------------------------------------------.
||       ||||'-------------------------------------------.         |
||       |||'-----------------------------.              |         |
||       ||'--------------------.         |              |         |
||       |'-------.             |         |              |         |
||       v         v            v         v              v         v
||    dir A                  dir B                    dir C
||    block 3   block 4      block 5   block 6        block 7   block 8
||  .---------.---------.  .---------.---------.    .---------.---------.
|'->| rev: 1  | rev: 0  |->| rev: 1  | rev: 2  |--->| rev: 1  | rev: 0  |
'-->|         |         |->|         | corrupt!|--->|         |         |
    |         |         |  |         | corrupt!| .->|         |         |
    |         |         |  |         | corrupt!| |  |         |         |
    '---------'---------'  '---------'---------' |  '---------'---------'
                                       block 9   |
                                     .---------. |
                                     | rev: 2  |-'
                                     |         |
                                     |         |
                                     |         |
                                     '---------'

|  update root directory to contain block 9
v

        root dir
        block 1   block 2
      .---------.---------.
      | rev: 1  | rev: 2  |--.
      |         |         |-.|
.-----|         |         |-|'
|.----|         |         |-'
||    '---------'---------'
||       .--------'||||'----------------------------------------------.
||       |         |||'-------------------------------------.         |
||       |         ||'-----------------------.              |         |
||       |         |'------------.           |              |         |
||       |         |             |           |              |         |
||       v         v             v           v              v         v
||    dir A                   dir B                      dir C
||    block 3   block 4       block 5     block 9        block 7   block 8
||  .---------.---------.   .---------. .---------.    .---------.---------.
|'->| rev: 1  | rev: 0  |-->| rev: 1  |-| rev: 2  |--->| rev: 1  | rev: 0  |
'-->|         |         |-. |         | |         |--->|         |         |
    |         |         | | |         | |         | .->|         |         |
    |         |         | | |         | |         | |  |         |         |
    '---------'---------' | '---------' '---------' |  '---------'---------'
                          |               block 6   |
                          |             .---------. |
                          '------------>| rev: 2  |-'
                                        | corrupt!|
                                        | corrupt!|
                                        | corrupt!|
                                        '---------'

|  remove corrupted block from linked-list
v

        root dir
        block 1   block 2
      .---------.---------.
      | rev: 1  | rev: 2  |--.
      |         |         |-.|
.-----|         |         |-|'
|.----|         |         |-'
||    '---------'---------'
||       .--------'||||'-----------------------------------------.
||       |         |||'--------------------------------.         |
||       |         ||'--------------------.            |         |
||       |         |'-----------.         |            |         |
||       |         |            |         |            |         |
||       v         v            v         v            v         v
||    dir A                  dir B                  dir C
||    block 3   block 4      block 5   block 9      block 7   block 8
||  .---------.---------.  .---------.---------.  .---------.---------.
|'->| rev: 1  | rev: 2  |->| rev: 1  | rev: 2  |->| rev: 1  | rev: 0  |
'-->|         |         |->|         |         |->|         |         |
    |         |         |  |         |         |  |         |         |
    |         |         |  |         |         |  |         |         |
    '---------'---------'  '---------'---------'  '---------'---------'
```

Also one question I've been getting is, what about the root directory?
It can't move so wouldn't the filesystem die as soon as the root blocks
develop errors? And you would be correct. So instead of storing the root
in the first few blocks of the storage, the root is actually pointed to
by the superblock. The superblock contains a few bits of static data, but
outside of when the filesystem is formatted, it is only updated when the root
develops errors and needs to be moved.
另一个问题，根目录如何呢？
它不能移动，那么文件系统不会在根块出现错误时立即死亡？你对了。
不同于将根存储在前面几个块，根实际上被超级块所指
超级快包含几个static数据，除了文件系统被格式化，这些static数据仅仅（当根出现错误需要移动时）被更新



## Wear leveling磨损均衡

The second concern for the littlefs is that blocks in the filesystem may wear
unevenly. In this situation, a filesystem may meet an early demise where
there are no more non-corrupted blocks that aren't in use. It's common to
have files that were written once and left unmodified, wasting the potential
erase cycles of the blocks it sits on.
LFS第二个关心的是文件系统中的块可能么有磨损均衡。在这种情况下，文件系统会出现过早挂掉（真正的挂掉：没有更多未损坏的块，且没使用的块）。文件写了一次就不更改很常见，浪费了这个块可擦除的潜力。


Wear leveling is a term that describes distributing block writes evenly to
avoid the early termination of a flash part. There are typically two levels
磨损均衡是一个

of wear leveling:
1. Dynamic wear leveling - Wear is distributed evenly across all **dynamic**
   blocks. Usually this is accomplished by simply choosing the unused block
   with the lowest amount of wear. Note this does not solve the problem of
   static data.
2. Static wear leveling - Wear is distributed evenly across all **dynamic**
   and **static** blocks. Unmodified blocks may be evicted for new block
   writes. This does handle the problem of static data but may lead to
   wear amplification.

In littlefs's case, it's possible to use the revision count on metadata pairs
to approximate the wear of a metadata block. And combined with the COW nature
of files, littlefs could provide your usual implementation of dynamic wear
leveling.

However, the littlefs does not. This is for a few reasons. Most notably, even
if the littlefs did implement dynamic wear leveling, this would still not
handle the case of write-once files, and near the end of the lifetime of a
flash device, you would likely end up with uneven wear on the blocks anyways.

As a flash device reaches the end of its life, the metadata blocks will
naturally be the first to go since they are updated most often. In this
situation, the littlefs is designed to simply move on to another set of
metadata blocks. This travelling means that at the end of a flash device's
life, the filesystem will have worn the device down nearly as evenly as the
usual dynamic wear leveling could. More aggressive wear leveling would come
with a code-size cost for marginal benefit.


One important takeaway to note, if your storage stack uses highly sensitive
storage such as NAND flash, static wear leveling is the only valid solution.
In most cases you are going to be better off using a full [flash translation layer (FTL)](https://en.wikipedia.org/wiki/Flash_translation_layer).
NAND flash already has many limitations that make it poorly suited for an
embedded system: low erase cycles, very large blocks, errors that can develop
even during reads, errors that can develop during writes of neighboring blocks.
Managing sensitive storage such as NAND flash is out of scope for the littlefs.
The littlefs does have some properties that may be beneficial on top of a FTL,
such as limiting the number of writes where possible, but if you have the
storage requirements that necessitate the need of NAND flash, you should have
the RAM to match and just use an FTL or flash filesystem.

## Summary摘要

So, to summarize:

1. The littlefs is composed of directory blocks
2. Each directory is a linked-list of metadata pairs
3. These metadata pairs can be updated atomically by alternating which
   metadata block is active
4. Directory blocks contain either references to other directories or files
5. Files are represented by copy-on-write CTZ skip-lists which support O(1)
   append and O(n log n) reading
6. Blocks are allocated by scanning the filesystem for used blocks in a
   fixed-size lookahead region that is stored in a bit-vector
7. To facilitate scanning the filesystem, all directories are part of a
   linked-list that is threaded through the entire filesystem
8. If a block develops an error, the littlefs allocates a new block, and
   moves the data and references of the old block to the new.
9. Any case where an atomic operation is not possible, mistakes are resolved
   by a deorphan step that occurs on the first allocation after boot

That's the little filesystem. Thanks for reading!
