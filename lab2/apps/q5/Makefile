default:
	cd makeprocs; make
	cd inject_h2o; make
	cd inject_so4; make
	cd reaction1; make
	cd reaction2; make
	cd reaction3; make

clean:
	cd makeprocs; make clean
	cd inject_h2o; make clean
	cd inject_so4; make clean
	cd reaction1; make clean
	cd reaction2; make clean
	cd reaction3; make clean
run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 2 1; ee469_fixterminal
