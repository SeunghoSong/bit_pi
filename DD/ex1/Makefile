#커널의 소스 경로
KDIR=/lib/modules/$(shell uname -r)/build
#모듈은 커널 시스템과 같은 버젼이거나 낮아야 한다. (높을경우 적재자체가 안된다.) 
obj-m=helloModule.o

default:
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	
