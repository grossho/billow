

all:
	rm -rf build bdist
	python setup.py build

install: all
	python setup.py install

clean:
	rm -rf build bdist *.core
