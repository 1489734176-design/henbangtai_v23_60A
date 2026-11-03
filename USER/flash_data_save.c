#include "flash_data_save.h"
#include "user_control.h"
#define FLASH_START_ADDRESS     ((uint32_t)0x08000000)
#define FLASH_PAGE_SIZE         (1024)
#define OffsetAddress         (FLASH_START_ADDRESS + 31 * FLASH_PAGE_SIZE)

#define DATA_BLOCK_SIZE      20  //数据块大小(半字数)���ݿ��С
#define DATA_BLOCK_NUM      10   //数据块数量（写入次数）���ݿ���� ��д������


#define BALANCE_EN  (1)

uint16_t save_data_buf[DATA_BLOCK_SIZE];
//void write_half_data(uint32_t Address,uint16_t write_data)
//{
//		uint32_t real_address;
//		real_address=OffsetAddress+Address;
//    FLASH_Unlock();

//    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

//    FLASH_ErasePage(real_address);
//    FLASH_ClearFlag(FLASH_FLAG_EOP);

//    FLASH_ProgramHalfWord(real_address, write_data);
//    FLASH_ClearFlag(FLASH_FLAG_EOP);

//    FLASH_Lock();
//}

uint16_t read_half_data(uint32_t Address)
{
	  return(*(volatile uint16_t *)(Address));
}



#if(BALANCE_EN)
void save_data_to_flash(uint16_t* save_data,uint8_t length )
{
	uint16_t read_data_temp;
//	uint16_t write_data_temp;
	uint8_t empty_addr_start=0;
	uint8_t i=0;
	uint32_t w_ptr;
	 for(i=0;i<DATA_BLOCK_NUM;i++)  // 遍历所有数据块寻找空块
	{
	
	    w_ptr = OffsetAddress+DATA_BLOCK_SIZE*i;

		  read_data_temp=read_half_data(w_ptr);
			if(read_data_temp!=0x55AA)  // 首字不等于0x55AA表示空块
			{
				empty_addr_start=i*DATA_BLOCK_SIZE;
				break;
			}		
	}
	
			if(i==DATA_BLOCK_NUM)  // 所有块已满，需要擦除
			{
			    FLASH_Unlock();

					FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

					FLASH_ErasePage(OffsetAddress);  // 擦除整页
					FLASH_ClearFlag(FLASH_FLAG_EOP);
				  empty_addr_start=0;
			}
			
//			write_data_temp=0x55AA;
//			FLASH_ProgramHalfWord(empty_addr_start+OffsetAddress, write_data_temp);
		 FLASH_Unlock();
			for(i=0;i<length;i++)  // 逐半字写入
			{
				FLASH_ProgramHalfWord(2*i+empty_addr_start+OffsetAddress, save_data[i]);
			}
		 FLASH_Lock();
}


uint8_t read_data_for_flash(uint16_t *read_data,uint8_t length)
{
	uint16_t read_data_temp;
	uint8_t empty_addr_start=0;
	uint8_t empty_addr_last=0;
	uint8_t i=0;
	uint8_t ret_sta=0;
	 uint32_t w_ptr;
	 for(i=0;i<DATA_BLOCK_NUM;i++)
	{
	
	    w_ptr = OffsetAddress+DATA_BLOCK_SIZE*i;

		  read_data_temp=read_half_data(w_ptr);
			if(read_data_temp!=0x55AA)
			{
				empty_addr_start=i*DATA_BLOCK_SIZE;
				break;
			}				
	}
	
			if(i==DATA_BLOCK_NUM)
		{
			empty_addr_start=i*DATA_BLOCK_SIZE;
		}
		if(empty_addr_start==0)
		{
//			*read_data=0;
			ret_sta=0;
		}
		else 
		{
			empty_addr_last=empty_addr_start-DATA_BLOCK_SIZE;  // 最后一个已写入块
			for(i=0;i<length;i++)
			{
			
			read_data[i]=read_half_data(2*i+empty_addr_last+OffsetAddress);
			}
				
			if(read_data[0]==0x55AA)  // 校验首字确认数据有效
			{
				ret_sta=1;
			}
			else 
			{
			ret_sta=0;
			
			}
		}
		
		return ret_sta;
}
#else
void save_data_to_flash(uint16_t* save_data,uint8_t length )
{
		uint8_t i=0;
		uint32_t real_address;
		real_address=OffsetAddress;
		FLASH_Unlock();

		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

		FLASH_ErasePage(OffsetAddress);
		FLASH_ClearFlag(FLASH_FLAG_EOP);
		for(i=0;i<length;i++)
		{
			FLASH_ProgramHalfWord(real_address+2*i, save_data[i]);
		}
    FLASH_ClearFlag(FLASH_FLAG_EOP);


}


uint8_t read_data_for_flash(uint16_t *read_data,uint8_t length)
{
			uint8_t i=0;
			for(i=0;i<length;i++)
			{
				read_data[i]=read_half_data(2*i);
			}
			return 1;

}
#endif

void save_user_data(void)
{
	save_data_buf[0]=0x55AA;  // 有效数据标志
	save_data_buf[1]=user_list.gears_level;  // 挡位等级
	save_data_buf[2]=user_list.flag.bits.gAutoStopEn;  // 自动停机使能
	save_data_to_flash(save_data_buf,3);
}

void read_user_data(void)
{

	u8 res=0;
	res=read_data_for_flash(save_data_buf,3);
	if(res==0)
	{
		save_data_buf[1]=1;  // 默认挡位=1
		save_data_buf[2]=0;  // 默认自动停机=0
	}
		
	user_list.gears_level=save_data_buf[1];
	if(user_list.gears_level>1)
	{
		user_list.gears_level=1;
	}
	
//	user_list.gears_ccw_level=save_data_buf[2];
	if(save_data_buf[2]>1)
	{
		save_data_buf[2]=0;
	}
	user_list.flag.bits.gAutoStopEn=save_data_buf[2];
}

