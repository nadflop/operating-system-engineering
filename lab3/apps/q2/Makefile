default:
	cd makeprocs; make
	cd inject_s2; make
	cd inject_co; make
	cd reaction1; make
	cd reaction2; make
	cd reaction3; make

clean:
	cd makeprocs; make clean
	cd inject_s2; make clean
	cd inject_co; make clean
	cd reaction1; make clean
	cd reaction2; make clean
	cd reaction3; make clean
run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 2 8; ee469_fixterminal
