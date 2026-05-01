JAVAC := javac

ROOT_JAVA := $(wildcard *.java)
JAVA_DIRS := $(sort $(patsubst ./%,%,$(shell find . -mindepth 2 -type f -name "*.java" -printf "%h\n")))

.PHONY: all clean root $(JAVA_DIRS)

all: $(if $(ROOT_JAVA),root) $(JAVA_DIRS)

root: $(ROOT_JAVA)
	$(JAVAC) $(ROOT_JAVA)

$(JAVA_DIRS):
	$(JAVAC) $@/*.java

clean:
	find . -type f -name "*.class" -delete
