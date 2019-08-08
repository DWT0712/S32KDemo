#ifndef __CONFIG_H__
#define __CONFIG_H__

//Flash Memory Configure

//0x0000_0000-0x0001_FFFF 512KB
#define Flash_Entry 0x00000000
#define Flash_Size  0x80000

//0x0000_0000-0x0000_3FFF  16KB             // ���ڴ��bootloader�ĳ������
#define BootloaderProgramFlashAddressEntry  	     Flash_Entry
#define BootloaderProgramFlashSize                   0x6000


//0x0000_5000-0x0000_51FF 512B              // ���һЩ���ݽṹ�壬�����ʱ���
#define ParameterSection1FlashEntry                  (BootloaderProgramFlashAddressEntry+BootloaderProgramFlashSize)   
#define ParameterSection1FlashSize                   0x1000

//0x0000_5200-0x0000_5FFF 512B            // ��ʱû��ʹ��,����
#define ParameterSection2FlashEntry                  (ParameterSection1FlashEntry+ParameterSection1FlashSize)
#define ParameterSection2FlashSize                   0x200

//0x0000_6000-0x0007_FFFF 488K              // ���ڴ��System�ĳ������
#define SystemProgramAddressFlashEntry                 (ParameterSection2FlashEntry+ParameterSection2FlashSize)
#define SystemProgramAddressFlashSize                  (Flash_Size - BootloaderProgramFlashSize  - ParameterSection1FlashSize - ParameterSection2FlashSize)

//Ram Memory Configure
//0x1FFF_8000-0x1FFF_FFFF 32KB
#define Ram1_Entry 0x1FFF8000
#define Ram1_Size  0x8000

//0x2000_0000-0x2000_6FFF 28KB
#define Ram2_Entry 0x20000000
#define Ram2_Size  0x7000


/************ Parameter Section RAM ************/
//0x2000_0000-0x2000_03FF 1KB           // ���RDS Pi Af�б������ (RadioRdsPiAfListAddress)
#define ParameterSection0Ram1Entry     (Ram2_Entry)
#define ParameterSection0Ram1Size      0x400

//0x2000_0400-0x2000_05FF 512B          // ����System���򱣴�ǳ�����صı��� (paramInfo)
#define ParameterSection1Ram1Entry     (ParameterSection0Ram1Entry + ParameterSection0Ram1Size)
#define ParameterSection1Ram1Size      0x200

//0x2000_0600-0x2000_07FF 512B          // ����System���򱣴泵����صı���
#define ParameterSection2Ram1Entry     (ParameterSection1Ram1Entry + ParameterSection1Ram1Size)
#define ParameterSection2Ram1Size      0x200

//0x2000_0800-0x2000_081F 32B           // ���ڳ���乲��ı���(�ÿռ�Ϊ�����������ṩ�ռ����LPC_RTC->GPREGx�Ĵ���)
#define ParameterSection3Ram1Entry     (ParameterSection2Ram1Entry+ParameterSection2Ram1Size)
#define ParameterSection3Ram1Size      0x20


//0x2000_0000-0x2000_081F 2K32B         // ����ParameterSectionRam1Sizeռ�еĿռ�
#define ALLParameterSectionRam1Entry    (ParameterSection0Ram1Entry)
#define ALLParameterSectionRam1Size     (ParameterSection0Ram1Size + ParameterSection1Ram1Size + ParameterSection2Ram1Size + ParameterSection3Ram1Size)


/************ update RAM ************/
//0x1FFF_8000-0x1FFF_FFFF 32KB          // ������Ϊupdate����������ڴ�
#define UpdaterRamRam1Entry            Ram1_Entry
#define UpdaterRamRam1Size             Ram1_Size

//0x2000_0820-0x2000_481F 16KB          // ������Ϊupdate����Ĵ��
#define UpdaterRomRam2Entry            (ALLParameterSectionRam1Entry + ALLParameterSectionRam1Size)
#define UpdaterRomRam2Size             (0x4000) // ��ȥParameter Section Size

//0x2000_4820-0x2000_6FFF  9K992B       // ������Ϊupdate����������ڴ�
#define UpdaterRamRam2Entry            (UpdaterRomRam2Entry + UpdaterRomRam2Size)
#define UpdaterRamRam2Size             (Ram2_Size - UpdaterRomRam2Size - ALLParameterSectionRam1Size)


/************ bootloader RAM ************/
//0x1FFF_8000-0x1FFF_FFFF 32KB          // ������Ϊbootloader���е��ڴ�
#define BootloaderRam1Entry            Ram1_Entry
#define BootloaderRam1Size             Ram1_Size

//0x2000_4820-0x2000_6FFF  9K992B       // ������Ϊbootloader���е��ڴ�
#define BootloaderRam2Entry            UpdaterRamRam2Entry
#define BootloaderRam2Size             UpdaterRamRam2Size


/************ bootloaderEncrypt RAM ************/
//0x1FFF_8000-0x1FFF_FFFF 32KB          // ������Ϊbootloader_encrypt���е��ڴ�
#define BootloaderEncryptRam1Entry     Ram1_Entry
#define BootloaderEncryptRam1Size      Ram1_Size

//0x2000_4820-0x2000_6FFF  9K992B       // ������Ϊbootloader_encrypt���е��ڴ�
#define BootloaderEncryptRam2Entry     UpdaterRamRam2Entry
#define BootloaderEncryptRam2Size      UpdaterRamRam2Size


/************ System RAM ************/
//0x1FFF_8000-0x1FFF_FFFF 32KB          // ������Ϊsystem����������ڴ�
#define SystemProgramRam1Entry            Ram1_Entry
#define SystemProgramRam1Size             Ram1_Size

//0x2000_4820-0x2000_6FFF  25K992       // ������Ϊsystem����������ڴ�
#define SystemProgramRam2Entry            (ALLParameterSectionRam1Entry + ALLParameterSectionRam1Size)
#define SystemProgramRam2Size             (Ram2_Size  - ALLParameterSectionRam1Size) // ��ȥParameter Section Size

//all 

#define BootloaderProgramAddressEntry               BootloaderProgramFlashAddressEntry
#define BootloaderProgramSize                       BootloaderProgramFlashSize




#define ParameterSection1ConfigAddressEntry			ParameterSection1FlashEntry
#define ParameterSection1ConfigSize			        ParameterSection1FlashSize

#define ParameterSection2ConfigAddressEntry			ParameterSection2FlashEntry
#define ParameterSection2ConfigSize			        ParameterSection2FlashSize

#define UpdaterProgramAddressEntry                  UpdaterRomRam2Entry
#define UpdaterProgramSize                          UpdaterRomRam2Size

#define SystemProgramAddressEntry			        SystemProgramAddressFlashEntry
#define SystemProgramSize			                SystemProgramAddressFlashSize


#define FLASH_SECTOR_SIZE       FEATURE_FLS_PF_BLOCK_SECTOR_SIZE


//������SystemProgram������ָ��һ����ַ,ʹ��һ���ֽڵĴ�С���һ������,���ڱ�ʾ��ǰ�Ƿ��е�һ���ϵ����г���
#define BirthmarkAddress                            (SystemProgramAddressEntry+0x34)

// RAM space for radio rds af 0x2000_6A00 ~ 0x2000_6DFF	(size is 0x400)-1KB
#define RadioRdsPiAfListAddress			ParameterSection0Ram1Entry

#endif

