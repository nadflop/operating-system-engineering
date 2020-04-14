default:
	cd makeprocs; make
	cd test_fork; make

clean:
	cd makeprocs; make clean
	cd test_fork; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj; ee469_fixterminal