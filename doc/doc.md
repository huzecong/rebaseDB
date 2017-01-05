# 数据库课程项目报告

- 计45  胡泽聪  2014011417
- 计45  韦毅龙  2014011434

## 概述

我们基于Stanford CS346课程的Redbase实验框架，实现了一个简易的数据库管理系统。遵照Redbase和课程的要求，我们的系统分为四个部分：记录管理模块（RM）、索引模块（IX）、系统管理模块（SM），和查询解析模块（QL）。在完成所有基本要求的基础上，我们还实现了以下附加功能：

- 基于B+树的索引；
- 任意多表单的连接；
- 基于贪心策略的查询优化，并生成查询计划（query plan）。

我们使用git进行项目管理。可以通过[https://git.net9.org/wyl8899/rebase.git](https://git.net9.org/wyl8899/rebase.git)访问项目仓库。

## 功能与用法

下面我们分类列出系统支持的所有功能，以及使用方法。

### 编译和运行

首先从git项目仓库中下载完整源代码。我们的项目使用CMake编译，C++代码遵循C++14标准，因此需要使用支持C++14标准的编译器。

另外，我们的项目使用Google的glog库实现调试信息输出与日志记录功能，使用Google的gflags库处理命令行参数。在Ubuntu下，可以通过`apt-get install libgoogle-glog-dev:i386`安装glog库的32位版本（这个版本的glog库依赖gflags，因此不需要再单独安装）。对于无法直接通过包管理系统安装的环境，可以手动编译32位版本，这里不赘述编译步骤。

我们的数据库系统的具体的编译命令如下：

```bash
git clone https://git.net9.org/wyl8899/rebase.git
cd rebase
mkdir cmake-build && cd cmake-build
cmake ..
make dbcreate
make redbase
```

可以得到两个可执行文件，其中`redbase`是数据库系统，`dbcreate`是`redbase`会调用的用于创建数据库的程序，亦可单独运行。

执行`./redbase`即可运行数据库系统。系统支持以下命令行参数：

- `-v <level>`：输出调试信息，`<level>`代表信息详细级别；
- `-c <command_file>`：执行`<command_file>`中的SQL语句，然后退出。

如果没有指定`-c`参数，那么系统会进入交互模式，用户可以输入SQL语句然后执行，并查看执行结果。

### 系统管理部分

支持的系统管理相关的SQL语句如下：

- 创建数据库：

  ```sql
  CREATE DATABASE db_name;
  ```

- 删除数据库：

  ```sql
  DROP DATABASE db_name;
  ```

- 列出当前目录下的所有数据库：

  ```sql
  SHOW DATABASES;
  ```

- 打开数据库：

  ```sql
  USE db_name;
  ```

- 列出数据库中包含的所有表：

  ```sql
  SHOW TABLES;
  ```

- 创建表：

  ```sql
  CREATE TABLE book (
    id INT(10) NOT NULL,
    title CHAR(100) NOT NULL,
    authors CHAR(200),
    price FLOAT(10) NOT NULL,
    PRIMARY KEY (id)
  );
  ```

  支持整数（int）、字符串（char）以及浮点数（float）三种数据类型，支持设置单个主键（primary key）以及非空约束（not null）。

  对于字符串类型，括号中的数字代表字符串最大长度；对于整数和浮点数类型，括号中的数字代表输出时最多显示的位数。

- 删除表：

  ```sql
  DROP TABLE book;
  ```

- 列出表中所有项：

  ```sql
  PRINT book;
  ```

- 显示表的模式信息：

  ```sql
  DESC book;
  ```

### 索引部分

- 创建索引：

  ```sql
  CREATE INDEX book(price);
  ```

  可以为任意单个属性创建索引，不要求属性值唯一。

- 删除索引：

  ```sql
  DROP INDEX book(price);
  ```

### 查询解析部分

- 插入数据：

  ```sql
  INSERT INTO book VALUES
    (300001, 'Database System Concepts', 'A. Silberschatz, H. F. Korth, S. Sudarshan', 65.0),
    (300002, 'Unknown book', NULL, 123.4);
  ```

  插入时会对各属性进行数据合法性检查（包括非空约束检查），同时会对主键进行唯一性检查。

  单条`INSERT`语句最多插入1024个表项。

- 删除数据：

  ```sql
  DELETE FROM book WHERE price > 100.0 AND authors IS NULL;
  ```

- 修改数据：

  ```sql
  UPDATE book SET authors = 'Myself' WHERE authors IS NULL;
  ```

- 查询：

  ```sql
  SELECT book.title, orders.quantity FROM book, orders
   WHERE book.id = orders.book_id AND
         orders.quantity > 8;
  ```

  支持任意多表单的连接：

  ```sql
  SELECT name, quantity, pages FROM customer, book, orders
   WHERE customer.id = orders.customer_id AND
         book.id = orders.book_id;
  ```

### 其他

- 显示/不显示查询计划：

  ```sql
  queryplans ON;
  queryplans OFF;
  ```

  类似于SQLite中的`EXPLAIN QUERY PLAN`指令，在查询时会输出生成的查询计划。

- 退出系统：

  ```sql
  EXIT;
  ```

  亦可通过输入EOF（`Ctrl+D`）来退出。

## 实现

下面我们分别介绍四个模块各自的实现。模块的作用以及接口大致与Redbase相同，因此我们着重描述有差别的部分，以及一些重要的实现细节。

### 记录管理模块（RM）

由于我们基于Redbase的框架，因此没有使用课程下发的页式文件存储模块，而使用了Redbase框架当中提供了基本相同的功能的PF模块。

我们的RM模块接口和Redbase框架所要求的接口有一些不同，这主要是因为框架没有考虑对NULL的支持。经过权衡我们决定将对NULL的支持放在RM模块。这样创建文件的时候就必须提供必要的信息（哪些域可能为NULL），插入一条记录的时候也必须额外地指定哪些域是NULL。在实现完成之后，我们的FileScan模块就能在框架要求的条件基础上额外地支持形如`IS NULL`和`IS NOT NULL`的条件。

#### 文件清单

RM部分包含以下文件：

* `rm.h`：包含RM相关组件的声明
* `rm_error.c`：用于输出RM部分的错误信息
* `rm_filehandle.cc`：包含`RM_FileHandle`类，提供插入、删除、更新记录的接口
* `rm_filescan.cc`：包含`RM_FileScan`类，提供基于简单条件来扫描文件的接口
* `rm_internal.h`：包含一些仅在RM内部使用的类和工具函数
* `rm_manager.cc`：包含`RM_Manager`类，提供创建、打开、销毁文件的接口
* `rm_record.cc`：包含`RM_Record`类，一个该类实例即是一条记录
* `rm_rid.{h,cc}`：包含`RM_RID`类，指定文件中一条记录的位置
* `rm_test.cpp`：包含对RM部分内部的类的测试

#### 存储方式

一个由RM部分创建和管理的文件具有如下结构：

* 首页（第0页）是以`RM_FileHeader`结构体形式存储的文件信息，包括：

    * `recordSize`：一条记录的大小，单位为字节
    * `recordsPerPage`：每个页能存储的记录的个数
    * `firstFreePage`：有空闲空间的页组成的链表中的第一个页
    * `nullableNum`：存储的记录里可能为NULL的域的个数
    * `nullableOffsets`：存储的记录里可能为NULL的域的偏移量

* 其他页具有`RM_PageHeader`结构体形式的头部，剩余空间用于存储记录，头部包括：

    * `firstFreeRecord`：因删除而空闲的位置组成的链表中的第一个位置
    * `allocatedRecords`：当前从头开始有多少位置已经被分配
    * `nextFreePage`：有空闲空间的页组成的链表中的下一个页
    * `bitmap`：以单个bit为单位存储的信息，包括两种：每个位置是否存储有记录；每个记录中每个可能为NULL的域是否确实为NULL。这些信息共占用`(nullableNum + 1) * recordsPerPage`个bit

为了避免插入和删除引发大量数据的移动，我们充分使用了链表结构。一个页面内，可存储的位置从前往后分配，只有没有空闲的位置时才向后分配，而空闲的位置用链表的方式维护。具体地，页头部内存储着空闲位置的链表的头，而指向的位置由于是空闲的，因此就能够用于存储下一个空闲的位置，而一个非法的位置编号（我们使用了-1）标志着这个链表的结束。在插入和删除时，这些信息都被相应地更新。跨页面的层面上，所有拥有空闲位置的页面也是用类似方法进行维护的。

#### 主要接口

RM模块被上层模块使用的主要方式是扫描（scan）。首先使用`RM_FileScan::OpenScan`打开一个扫描，然后持续调用`RM_FileScan::GetNextRec`获取下一条记录，直至返回值为`RM_EOF`。`RM_FileScan::OpenScan`允许指定一个偏移量`offset`，一个类型`type`，一个值的指针`value`和一个运算符`op`，只有符合`*(type *)(r + offset) op *(type *)value`的记录`r`才会在`GetNextRec`调用当中被获取到。

### 索引模块（IX）

IX模块在页式文件上实现了简单的B+树，能够支持将各种类型（整数，浮点数，以及长度有上限的字符串）作为键值，同时支持一个键值对应多个值。相对于通用的B+树，这里的B+树当中存储的值的类型固定为RID，一个键值对应到的RID保存在同一个页（称为bucket）当中，因此其个数不能多于一个页能存下的个数，此外，删除操作永远不会删除节点而最多只会回收节点对应的存储RID的页（如果删除之后没有别的RID）。这些简化降低了实现上的难度。

#### 文件清单

- `ix.h`：包含IX相关组件的声明
- `ix_error.cc`：用于输出IX部分的错误信息
- `ix_indexhandle.cc`：包含`IX_IndexHandle`类，提供插入、删除项的接口
- `ix_indexscan.cc`：包含`IX_IndexScan`类，提供使用简单条件来扫描索引的接口
- `ix_internal.h`：包含一些仅在IX内部使用的类
- `ix_manager.cc`：包含`IX_Manager`类，提供创建、打开、销毁索引的接口
- `ix_test.cpp`：包含对IX部分内部的类的测试

#### 存储方式

一个由IX模块创建和管理的索引文件具有如下结构：

- 首页是以`IX_FileHeader`结构体（见`ix_internal.h`）形式存储的文件信息，包括
  - `attrType`：索引的属性的类型（整数，浮点数，字符串）
  - `attrLength`：索引的属性的大小（单位为字节），对于整数和浮点数来说总是4
  - `root`：B+树根节点所在的页号
- 节点页开头以`IX_PageHeader`形式存储如下信息
  - `type`：表示该节点是内部节点还是叶节点
  - `childrenNum`：该节点的孩子的数目
  - 余下空间存储形如 `{ int pageNum; char key[]; }`的项，`pageNum`在内部节点表示子节点所在页编号，在叶子节点表示存储其对应的bucket所在的页编号；`key`存储着键值，符合B+树中的定义。叶子节点的最后一个项的`pageNum`存储着下一个叶子节点所在的页编号，使得可以跟着这个编号连续地访问从某节点开始的所有叶节点
- bucket页开头4字节表明该bucket存储的RID的个数，其余空间用来存储RID；在bucket当中RID不分先后，其插入和删除是朴素的，可能涉及到多个元素的移动。

### 系统管理模块（SM）

系统管理模块负责创建、删除，和维护数据库。

SM会维护数据库中各表以及表中的字段的信息（即模式信息），这一信息本身也是作为数据库中的表存储的。表`relcat`存储了数据库中的各个表的信息，表`attrcat`存储了各表中各属性的信息。在插入、删除或修改记录的时候，如果发生了对所维护信息的修改，则需要调用SM的接口更新信息。

#### 文件清单

SM部分包含下列文件：

- `sm.h`：包含SM相关组件的声明
- `sm_error.cc`：用于输出SM部分的错误信息
- `sm_manager.cc`：包含`SM_Manager`类，负责处理所有系统管理操作
- `catalog.h`：包含`relcat`和`attrcat`表项的定义，详情请见下文

#### 表单模式的存储

`relcat`中的表项存储了以下信息：

- `relName`：表单的名称
- `tupleLength`：表项占用的字节数（考虑内存对齐）
- `attrCount`：属性的数量
- `indexCount`：有索引的属性数量
- `recordCount`：表项的数量

`attrcat`中的表项存储了以下信息：

- `relName`：表单的名称
- `attrName`：属性的名称
- `offset`：属性存储位置相对于表项开头的偏移量
- `attrType`：属性的数据类型
- `attrSize`：属性占用的字节数（考虑内存对齐）
- `attrDisplaySize`：属性的显示长度（对字符串来说，即其最长长度；对整数来说，即其最多显示的位数）
- `attrSpecs`：属性的限制，以各个二进制位是否为1表示是否具有对应的限制，限制有：
  - `ATTR_SPEC_NOTNULL`：属性具有非空约束
  - `ATTR_SPEC_PRIMARYKEY`：属性是主键
- `indexNo`：属性的索引编号，如果不存在索引则为-1

#### 主要接口

SM提供了三个接口以供QL等模块获取相关信息：

- `SM_Manager::GetRelEntry`：获得指定表单在`relcat`中的表项
- `SM_Manager::GelAttrEntry`：获得指定属性在`attract`中的表项
- `SM_Manager::GetDataAttrInfo`：获得指定表单的所有属性的信息，返回的是一个`DataAttrInfo`类型的数组。这一类型相较于`attrcat`表项，额外存储了`nullableIndex`，代表属性在空值位图中的下标（若字段具有非空约束，则为-1）

另外还有`SM_Manager::UpdateRelEntry`以及`SM_Manager::UpdateAttrEntry`以实现对信息表项的更新。

### 查询解析模块（QL）

查询解析部分负责处理SQL语句中的`SELECT`、`INSERT`、`DELETE`和`UPDATE`语句，然后对数据库进行修改，或者将查询结果输出到屏幕。

#### 文件清单

QL部分包含下列文件：

- `ql.h`：包含QL相关组件的声明
- `ql_disjoint.{h,cc}`：并查集数据结构的实现，在生成查询计划时使用
- `ql_error.cc`：用于输出QL部分的错误信息
- `ql_internal.h`：包含仅在QL内部使用的一些类和函数
- `ql_iterator.{h,cc}`及所有的`ql_*iter.cc`：使用迭代器模式的查询部件，详情请见下文
- `ql_manager.cc`：包含`QL_Manager`类，负责所有的查询解析操作

#### 插入、删除、修改操作

这一部分使用了最朴素的实现。

对于插入操作，首先检查插入数据的合法性，包括数据类型是否匹配，非空约束是否满足，以及主键是否重复。如果合法，那么调用RM的接口完成数据的写入，同时对于创建了索引的属性，调用IX的借口插入新的索引。

对于删除操作，首先判断条件是否合法。如果合法，那么遍历整个数据库，检查当前表项是否满足条件。如果满足，则进行删除。删除过程基本上和插入一致。

对于修改操作，同样判断合法性。如果合法，那么遍历整个数据库，检查当前表项是否满足条件。如果满足，那么修改其对应属性的值。如果属性具有索引，那么需要先删除原有的索引项，再插入修改后的索引项。

#### 查询计划的生成

对于涉及多个表的查询，最简单的方法自然是使用若干个嵌套循环，枚举每个表中的每个表项，然后判断是否满足条件。但这一方法的速度太慢，对于具有$m$个条件，涉及$k$个表，平均每个表具有$n$个表项的查询，复杂度为$O(mn^k)$。

我们使用一种贪心策略来生成查询计划。这一策略基于以下的断言与准则：

- 表中每个属性的取值分布是均匀的；
- 尽可能利用索引的信息；
- 尽早地减少候选表项的数量和属性的数量。

在介绍策略之前，先做如下的定义：

- 如果一个条件涉及来自两个表的不同属性，那么称之为**复杂条件**，否则称之为针对某一个表的**简单条件**；
- 考虑一张表，称最终输出的属性，以及存在于所有条件中的属性的并集，与表的属性集合的交集为**投影集合**。

我们的策略如下：

1. 考虑所有涉及两个不同的表的条件`a.x op b.y`，其中`a`和`b`为表，`x`和`y`为属性，`op`为关系运算符
   - 如果表`a`尚未访问过，而且`a.x`具有索引，那么
     - 遍历（file scan）表`b`，如果其尚未访问过，那么使用其简单条件进行过滤（selection），再使用其投影集合进行投影（projection）
     - 对于上面得到的每一个表项，将其属性`y`的值作为索引关键字，使用`a.x`的索引（index scan）获得满足条件的`a`中的表项，使用其简单条件进行过滤（selection），再使用其投影集合进行投影（projection）
     - 将两个表项合并，找出复杂条件中属性已经被合并到同一个表的条件，使用这些条件进行过滤（selection）
   - 优先处理`op`为等号的条件：只有当不存在`op`为等号的条件时，才考虑`op`不为等号的条件
2. 考虑所有尚未访问的表
   - 如果其具有某个简单条件`a.x op rhs`，满足`a.x`具有索引，且`rhs`是值而非另一个属性
     - 将`rhs`作为索引关键字，使用`a.x`的索引（index scan）获得满足条件的表项，使用其他所有简单条件进行过滤（selection），再使用其投影集合进行投影（projection）
   - 否则
     - 遍历（file scan）表`a`，使用其简单条件进行过滤（selection），再使用其投影集合进行投影（projection）
3. 考虑尚未处理的复杂条件
   - 使用朴素的嵌套循环方法合并两个表
   - 找出复杂条件中属性已经被合并到同一个表的条件，使用这些条件进行过滤（selection）
4. 使用最终输出的属性集合进行投影（projection）

需要注意的是，在第1步和第3步中，合并的两个表不一定是数据库中的表单，而可能是由之前的合并操作产生的合并后的表单。

为了提升效率，在判断两个表是否已经被合并时，使用了并查集数据结构。

#### 迭代器模式

为了方便地执行查询计划，我们选择使用迭代器模式。即，我们将整个查询计划划分成流水线上的若干阶段，每一阶段由一个迭代器负责。一个迭代器具有一个或多个输入，当我们向一个迭代器请求数据的时候，迭代器会向其输入迭代器请求数据，对数据进行处理后，将其作为输出返回给我们。这有点类似于一些函数式语言中的惰性求值的行为。

具体来说，我们为查询计划中的每一个操作实现了一个迭代器类。所有迭代器类都是`QL_Iterator`类的派生类。`QL_Iterator`具有三个接口：

- `GetNextRec`：获取下一个表项
- `Reset`：回到第一个表项，在合并两个表时的需要用到
- `Print`：输出迭代器的信息

我们实现了以下的迭代器：

- `QL_FileScanIterator`：打开并遍历整个表单
  - 不具有输入迭代器。
  - 对于`GetNextRec`，使用`RM_FileScan`从文件中读取下一个表项。
  - 对于`Reset`，关闭并重新打开`RM_FileScan`。
- `QL_SelectionIterator`：使用条件进行过滤
  - 具有一个输入迭代器。
  - 对于`GetNextRec`，不断向输入迭代器请求数据，直到找到一个满足所有条件的表项为止。
  - 对于`Reset`，要求其输入迭代器进行`Reset`。
- `QL_ProjectionIterator`：使用属性集合进行投影
  - 具有一个输入迭代器。
  - 对于`GetNextRec`，向输入迭代器请求数据，生成投影之后的表项。
  - 对于`Reset`，要求其输入迭代器进行`Reset`。
- `QL_IndexSearchIterator`：使用索引进行条件遍历
  - 不具有输入迭代器。
  - 对于`GetNextRec`，使用`IX_IndexScan`从索引中获取下一个索引，并从文件读取对应的表项。
  - 对于`Reset`，关闭并重新打开`IX_IndexScan`。
  - 额外具有一个`ChangeValue`接口，可以修改作为索引条件的值，供`QL_IndexedJoinIterator`使用。
- `QL_NestedLoopJoinIterator`：嵌套循环表单合并
  - 具有两个输入迭代器。
  - 对于`GetNextRec`，尝试从第2个输入迭代器请求数据，然后将两个表项合并；如果第2个输入迭代器已经完成遍历，则从第1个输入迭代器请求数据，然后要求第2个输入迭代器进行`Reset`，并再次请求数据。
  - 对于`Reset`，要求其两个输入迭代器进行`Reset`。
- `QL_IndexedJoinIterator`：基于索引的表单合并
  - 具有三个输入迭代器：其中两个用于请求数据，另外一个是`QL_IndexSearchIterator`。
  - 对于`GetNextRec`，类似嵌套循环合并，但可能需要反复进行；同时，在获取第1个输入迭代器的数据后，先修改`QL_IndexSearchIterator` 的索引条件的值，再进行`Reset`。
  - 对于`Reset`，要求其两个输入迭代器进行`Reset`。

使用迭代器模式的好处有：

- 遵循模块化设计原则，为查询计划的生成带来方便；
- 具有很强的可拓展性；
- 可以方便地并行化，以进一步提升效率。

#### 测试用例

下面展示一些查询生成的查询计划。表单的模式信息与所提供的测试数据中的表单相同。

具有索引的属性有：`customer.id`、`orders.quantity`、`book.id`、`book.pages`，和`publisher.id`。

##### 测试用例1

SQL语句：

```sql
SELECT name, quantity, pages FROM customer, orders, book
 WHERE customer.id = orders.customer_id AND
       book.id = orders.book_id AND
       pages > 100 AND
       quantity > 5;
```

查询计划：

```
9: PROJECTION name quantity pages
└──8: INDEXED JOIN 4 and 7
   ├──4: INDEXED JOIN 3 and 2
   │  ├──3: SEARCH orders.quantity > 5
   │  └──2: PROJECTION id name
   │     └──1: SEARCH customer.id = orders.customer_id
   └──7: PROJECTION id pages
      └──6: SELECTION book.pages > 100
         └──5: SEARCH book.id = orders.book_id
```

##### 测试用例2

SQL语句：

```sql
SELECT * FROM customer, publisher, orders, book
 WHERE customer.gender = 'F' AND
       customer.id = orders.customer_id AND
       pages > 100 AND
       state = 'CA' AND
       publisher.id = book.publisher_id;
```

查询计划：

```
10: PROJECTION *
└──9: NESTED-LOOP JOIN 4 and 8
   ├──4: INDEXED JOIN 3 and 2
   │  ├──3: SCAN orders
   │  └──2: SELECTION customer.gender = F
   │     └──1: SEARCH customer.id = orders.customer_id
   └──8: INDEXED JOIN 7 and 6
      ├──7: SEARCH book.pages > 100
      └──6: SELECTION publisher.state = CA
         └──5: SEARCH publisher.id = book.publisher_id
```

## 总结

经过一学期的学习和努力，我们终于完成了这份大作业。在迄今为止做过的大作业中，数据库大作业的工作量和代码量都不算小。经过统计，排除掉提供的框架，最后我们写了超过5000行，约200KB的代码。

比较遗憾的是，出于时间与精力的限制，我们还有许多想要实现的功能没来得及实现，包括聚集与排序；同时，目前版本的代码也还有优化的余地。

很幸运选上了这门课，得以了解数据库背后的技术，得以完成这个大作业。感谢两位老师的讲授，也感谢助教的辛勤付出。