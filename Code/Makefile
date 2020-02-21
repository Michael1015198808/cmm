# GNU make手册：http://www.gnu.org/software/make/manual/make.html
# ************ 遇到不明白的地方请google以及阅读手册 *************

# 编译器设定和编译选项
CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -std=c99 -g

# 编译目标：src目录下的所有.c文件
CFILES = $(shell find ./ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find ./ -name "*.l")
YFILE = $(shell find ./ -name "*.y")
LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

TEST_IN = $(shell find ../Test -name "*.cmm")
TEST_CASE = $(TEST_IN:.cmm=)
TEST_OUT = $(TEST_IN:.cmm=.out)

parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o parser $(filter-out $(LFO),$(OBJS)) -lfl -ly
	@cp parser ../

syntax: lexical syntax-c
	$(CC) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

# 定义的一些伪目标
.PHONY: clean test gdb gen runall 
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *~
#gdb运行parser
gdb: parser
	@gdb parser
#一键测试../Test目录下所有.cmm文件的输出是否和.out一致
test:
	@for case in $(TEST_CASE); \
	do \
		./parser $$case.cmm > temp.out; \
		if diff temp.out $$case.out >/dev/null; \
		then true; \
		else echo Wrong on $$case;\
	fi; \
	done;
#生成../Test目录下所有.cmm文件对应的.out，以备日后make test
gen: parser $(TEST_OUT)

../Test/%.out : ../Test/%.cmm parser
	./parser $< > $@
#打印../Test目录下所有.cmm文件并生成对应.out
runall: parser $(TEST_OUT)
	@for case in $(TEST_CASE); \
	do \
	echo $$case.cmm; \
	cat $$case.cmm; \
	echo $$case.out; \
	cat $$case.out; \
	done;
