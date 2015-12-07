OTH_PATH = ../../3rd_party

AR      = ar 
ARFLAGS = -cr

CXX      = g++
INC      = -I./include 							\
		   -I$(OTH_PATH)/eagle/include  		\
		   -I$(OTH_PATH)/eagle_tool/include     \
		   -I$(OTH_PATH)/log4plus/include 		\
		   -I$(OTH_PATH)/urcu/include   		\
		   -I$(OTH_PATH)/log/include 			\
		   -I$(OTH_PATH)/client_socket/include 	\
		   -I$(OTH_PATH)/libconfig/include 		\
		   $(NULL)
		   
CXXFLAGS = $(INC) -static -Wall -g


# makefile��ȱʡĿ��
target = libraft.a
.PHONY : all 
all : $(target)
	rm -rf lib
	mkdir lib
	mv $(target) lib

sources = src/raft.cc 					\
		  src/raft_global.cc 			\
		  src/raft_receive_work.cc 		\
		  src/raft_handle_work.cc 		\
		  src/raft_leader_work.cc 		\
		  src/raft_candidate_work.cc 	\
		  src/raft_follower_work.cc 	\
		  src/raft_send_work.cc 		\
		  src/raft_cinfo_handle_work.cc \
		  src/raft_cinfo_send_work.cc 	\
		  src/raft_notify_client_work.cc

objs := $(sources:%.cc=%.o)

# Ŀ���ļ������������ɵ�����
$(target) : $(objs)
	$(AR) $(ARFLAGS) $@ $^

# .o�ļ������������ɵ�����
%.o : %.cc
	$(CXX) -c $< -o $@ $(CXXFLAGS)

.PHONY : clean
clean :
	rm -rf lib
	find ./ -name '*~'  -exec rm -rf {} \;
	find ./ -name '*.o' -exec rm -rf {} \;

