CC= g++
LIBS= -lomniORB4 -lomnithread -lomniDynamic4
FLAGS= -g -I/opt/include -L/opt/lib
IDLFILES=Echo.idl
HEADERS=Echo_i.hh
CPPCLIENTFILES= EchoSK.cc eg2_clt.cc
CPPIMPLFILES=EchoSK.cc eg2_impl.cc Echo_i.cc
IMPL=echoSrv
CLIENT= echoClient

#g++ -g -I/opt/include EchoSK.cc eg1.cc -lomniORB4 -lomnithread -lomniDynamic4 -L/opt/lib -I/opt/include


ALL: $(CPPIMLPFILES) $(CPPCLIENTFILES) $(HEADERS)
	$(CC) $(FLAGS) $(CPPIMPLFILES) $(LIBS) -o $(IMPL)
	$(CC) $(FLAGS) $(CPPCLIENTFILES) $(LIBS) -o $(CLIENT)

IDL: $(IDLFILES)
	omniidl -bcxx $(IDLFILES)
CLEAN:
	rm *.o
#compiler flags taken from: http://www.yolinux.com/TUTORIALS/CORBA.html
#code example from: http://www.omniorb-support.com/omni42/omniORB/index.html
