#include "iarduino_I2C_Expander.h"																						//
																														//
//		Инициализация расширителя выводов:																				//	Возвращаемое значение: результат инициализации.
bool	iarduino_I2C_Expander::begin			(void){																	//	Параметр: отсутствует
			uint8_t j;																									//	Объявляем переменную для сравнения идентификатора чипа.
		//	Инициируем работу с шиной I2C:																				//
			objI2C->begin(100);																							//	Инициируем передачу данных по шине I2C на скорости 100 кГц.
		//	Если адрес не указан, то ищим модуль на шине I2C:															//
			if(valAddrTemp==0){																							//
				for(int i=1; i<127; i++){																				//	Проходим по всем адресам на шине I2C
					if( objI2C->checkAddress(i)										){									//	Если на шине I2C есть устройство с адресом i, то ...
					if( objI2C->readByte(i,REG_MODEL)          == DEF_MODEL_EXP		){									//	Если у модуля с адресом i в регистре «MODEL»   хранится значение DEF_MODEL_EXP     ( расширитель выводов ), то ...
					j = objI2C->readByte(i,REG_CHIP_ID); if( j == DEF_CHIP_ID_FLASH ||	j==DEF_CHIP_ID_METRO	){		//	Если у модуля с адресом i в регистре «CHIP_ID» хранится значение DEF_CHIP_ID_FLASH (идентификатор модулей Flash), или DEF_CHIP_ID_METRO (идентификатор модулей Metro), то ...
					if( objI2C->readByte(i,REG_ADDRESS)>>1     == i					){	valAddrTemp=i; i=128;	}else	//	Если у модуля с адресом i в регистре «ADDRESS» хранится значение i                 ( адрес + младший бит ), то cчитаем что модуль обнаружен, сохраняем значение i как найденный адрес и выходим из цикла.
					if( objI2C->readByte(i,REG_ADDRESS)        == 0xFF				){	valAddrTemp=i; i=128;	}		//	Если у модуля с адресом i в регистре «ADDRESS» хранится значение 0xFF              ( адрес не задавался  ), то cчитаем что модуль обнаружен, сохраняем значение i как найденный адрес и выходим из цикла.
					}																									//
					}																									//
					}																									//
				}																										//
			}																											//
		//	Если модуль не найден, то возвращаем ошибку инициализации:													//
			if( valAddrTemp == 0																			){return false;}	//
		//	Проверяем наличие модуля и значения его регистров:																	//
			if( objI2C->checkAddress(valAddrTemp)            == false										){return false;}	//	Если на шине I2C нет устройств с адресом valAddrTemp,                     то возвращаем ошибку инициализации
			if( objI2C->readByte(valAddrTemp,REG_MODEL)      != DEF_MODEL_EXP								){return false;}	//	Если значение регистра «MODEL»   не совпадает со значением DEF_MODEL_EXP, то возвращаем ошибку инициализации
			j = objI2C->readByte(valAddrTemp,REG_CHIP_ID); if( j!=DEF_CHIP_ID_FLASH && j!=DEF_CHIP_ID_METRO	){return false;}	//	Если значение регистра «CHIP_ID» не совпадает со значением DEF_CHIP_ID_FLASH и DEF_CHIP_ID_METRO, то возвращаем ошибку инициализации
			if( objI2C->readByte(valAddrTemp,REG_ADDRESS)>>1 != valAddrTemp									){					//	Если значение регистра «ADDRESS» не совпадает с адресом модуля,           то ...
			if( objI2C->readByte(valAddrTemp,REG_ADDRESS)    != 0xFF										){return false;}}	//	Если значение регистра «ADDRESS» не совпадает со значением 0xFF,          то возвращаем ошибку инициализации
		//	Определяем значения переменных:																				//	
			valAddr=valAddrTemp;																						//	Сохраняем адрес модуля на шине I2C.
			valVers=objI2C->readByte(valAddr,REG_VERSION);																//	Сохраняем версию прошивки модуля.
		//	Перезагружаем модуль устанавливая его регистры в значение по умолчанию:										//
			reset();																									//	Выполняем программную перезагрузку.
			delay(5);																									//
			return true;																								//	Возвращаем флаг успешной инициализаии.
}																														//
																														//
//		Смена адреса модуля:																							//	Возвращаемое значение:	резульат смены адреса.
bool	iarduino_I2C_Expander::changeAddress	(uint8_t newAddr){														//	Параметр:				newAddr - новый адрес модуля.
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Проверяем новый адрес:																						//
			if(newAddr>0x7F){newAddr>>=1;}																				//	Корректируем адрес, если он указан с учётом бита RW.
			if(newAddr==0x00 || newAddr==0x7F){return false;}															//	Запрещаем устанавливать адрес 0x00 и 0x7F.
		//	Записываем новый адрес:																						//
			if(_readBytes(REG_BITS_0,1)==false){return false;}															//	Читаем 1 байт регистра «BITS_0» в массив «data».
			data[0] |= 0b00000010;																						//	Устанавливаем бит «SET_PIN_ADDRES»
			if(_writeBytes(REG_BITS_0,1)==false){return false;}															//	Записываем 1 байт в регистр «BITS_0» из массива «data».
			data[0] = (newAddr<<1)|0x01;																				//	Готовим новый адрес к записи в модуль.
			if(_writeBytes(REG_ADDRESS,1)==false){return false;}														//	Записываем 1 байт в регистр «ADDRESS» из массива «data».
			delay(200);																									//	Даём более чем достаточное время для применения модулем нового адреса.
		//	Проверяем наличие модуля с новым адресом на шине I2C:														//
			if(objI2C->checkAddress(newAddr)==false){return false;}														//	Если на шине I2C нет модуля с адресом newAddr, то возвращаем ошибку.
			valAddr = newAddr;																							//	Сохраняем новый адрес как текущий.
			return true;																								//	Возвращаем флаг успеха.
		}else{																											//	Иначе, если расширитель не инициализирован, то ...
			return false;																								//	Возвращаем ошибку
		}																												//
}																														//
																														//
//		Перезагрузка модуля:																							//	Возвращаемое значение:	результат перезагрузки.
bool	iarduino_I2C_Expander::reset			(void){																	//	Параметр:				отсутствует.
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Сбрасываем глобальные переменные в значения по умолчанию:													//	
			valFreqPWM = 490;																							//	Сбрасываем установленную частоту ШИМ в значение по умолчанию.
			for(uint8_t i=0; i<8; i++){ arrMode[i] = 0; }																//	Сбрасываем режим работы выводов в значение по умолчанию.
			for(uint8_t i=0; i<4; i++){ arrServ[i][0]=500; arrServ[i][1]=2500; arrServ[i][2]=0; arrServ[i][3]=180; } 	//	Сбрасываем соотношение длительности к углу сервопривода в значение по умолчанию.
		//	Устанавливаем бит перезагрузки:																				//
			if(_readBytes(REG_BITS_0,1)==false){return false;}															//	Читаем 1 байт регистра «BITS_0» в массив «data».
			data[0] |= 0b10000000;																						//	Устанавливаем бит «SET_RESET»
			if(_writeBytes(REG_BITS_0,1)==false){return false;}															//	Записываем 1 байт в регистр «BITS_0» из массива «data».
		//	Ждём установки флага завершения перезагрузки:																//
			do{ if(_readBytes(REG_FLAGS_0,1)==false){return false;} }													//	Читаем 1 байт регистра «REG_FLAGS_0» в массив «data».
			while( (data[0]&0b10000000) == 0);																			//	Повторяем чтение пока не установится флаг «FLG_RESET».
			return true;																								//
		}else{																											//	Иначе, если расширитель не инициализирован, то ...
			return false;																								//	Возвращаем ошибку
		}																												//
}																														//
																														//
//		Конфигурирование выводов:																						//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::pinMode			(uint8_t pin, uint8_t dir, uint8_t type){								//	Параметры:				номер вывода, направление работы вывода, тип сигнала.
		if((pin<8)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
			uint8_t f=1, min=0, max=7; if(pin<8){min=max=pin;}															//	Определяем переменную «n» определяющую количество записываемых байт, а так же определяем минимальный и максимальный номер вывода.
		//	Читаем значение регистра «REG_EXP_DIRECTION» и следующего за ним регистра «REG_EXP_TYPE»:					//
			_readBytes(REG_EXP_DIRECTION, 2);																			//	Читаем 2 байта регистров «REG_EXP_DIRECTION» и «REG_EXP_TYPE» в массив «data».
		//	Обновляем данные:																							//
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				if( dir  == OUTPUT  )	{ data[0] |=  (1<<i);			arrMode[i] |=  EXP_BIT_DIR; }					//	Если вывод конфигурируется как выход       , то устанавливаем бит регистра «REG_EXP_DIRECTION» и устанавливаем бит «EXP_BIT_DIR» в массиве «arrMode».
				if( dir  == INPUT   )	{ data[0] &= ~(1<<i);			arrMode[i] &= ~EXP_BIT_DIR; }					//	Если вывод конфигурируется как вход        , то сбрасываем    бит регистра «REG_EXP_DIRECTION» и сбрасываем    бит «EXP_BIT_DIR» в массиве «arrMode».
				if( type == ANALOG  )	{ data[1] |=  (1<<i);	f=2;	arrMode[i] |=  EXP_BIT_TYP; }					//	Если вывод конфигурируется как аналоговый  , то устанавливаем бит регистра «REG_EXP_TYPE»      и устанавливаем бит «EXP_BIT_TYP» в массиве «arrMode».
				if( type == SERVO   )	{ data[1] |=  (1<<i);	f=2;	arrMode[i] |=  EXP_BIT_TYP; }					//	Если вывод конфигурируется для сервопривода, то устанавливаем бит регистра «REG_EXP_TYPE»      и устанавливаем бит «EXP_BIT_TYP» в массиве «arrMode».
				if( type == DIGITAL )	{ data[1] &= ~(1<<i);	f=2;	arrMode[i] &= ~EXP_BIT_TYP; }					//	Если вывод конфигурируется как цифровой    , то сбрасываем    бит регистра «REG_EXP_TYPE»      и сбрасываем    бит «EXP_BIT_TYP» в массиве «arrMode».
			}																											//
		//	Записываем обновлённые данные в регистр «REG_EXP_DIRECTION» [и «REG_EXP_TYPE»]:								//
			_writeBytes(REG_EXP_DIRECTION, f);																			//	Записываем 1 или 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_DIRECTION».
		//	Устанавливаем частоту ШИМ = 50 Гц для управления сервоприводами:											//
			if( type == SERVO ){ freqPWM(50); }																			//	Теперь, задавая угол поворота, модулю не будет отпавлена команда на смену частоты в 50 Гц.
		}																												//
		}																												//
}																														//
																														//
//		Подключение к выводу прижимающего или подтягивающего резистора:													//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::pinPull			(uint8_t pin, uint8_t pull){											//	Параметры:				номер вывода, резистор.
		if((pin<8)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
			uint8_t min=0, max=7; if(pin<8){min=max=pin;}																//	Определяем минимальный и максимальный номер вывода.
		//	Читаем значение регистра «REG_EXP_PULL_UP» и следующего за ним регистра «REG_EXP_PULL_DOWN»:				//
			_readBytes(REG_EXP_PULL_UP, 2);																				//	Читаем 2 байта регистров «REG_EXP_PULL_UP» и «REG_EXP_PULL_DOWN» в массив «data».
		//	Обновляем данные:																							//
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				if( pull == PULL_UP   )	{ data[0] |=  (1<<i);			arrMode[i] |=  EXP_BIT_PUP; }					//	Если вывод требуется подтянуть   , то устанавливаем бит регистра «REG_EXP_PULL_UP»   и устанавливаем бит «EXP_BIT_PUP» в массиве «arrMode».
				else					{ data[0] &= ~(1<<i);			arrMode[i] &= ~EXP_BIT_PUP; }					//	Иначе, если подтяжка не требуется, то сбрасываем    бит регистра «REG_EXP_PULL_UP»   и сбрасываем    бит «EXP_BIT_PUP» в массиве «arrMode».
				if( pull == PULL_DOWN )	{ data[1] |=  (1<<i);			arrMode[i] |=  EXP_BIT_PDN; }					//	Если вывод требуется прижать     , то устанавливаем бит регистра «REG_EXP_PULL_DOWN» и устанавливаем бит «EXP_BIT_PDN» в массиве «arrMode».
				else					{ data[1] &= ~(1<<i);			arrMode[i] &= ~EXP_BIT_PDN; }					//	Иначе, если прижатие не требуется, то сбрасываем    бит регистра «REG_EXP_PULL_DOWN» и сбрасываем    бит «EXP_BIT_PDN» в массиве «arrMode».
			}																											//
		//	Записываем обновлённые данные в регистр «REG_EXP_PULL_UP» и «REG_EXP_PULL_DOWN»:							//
			_writeBytes(REG_EXP_PULL_UP, 2);																			//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_PULL_UP».
		}																												//
		}																												//
}																														//
																														//
//		Выбор схемы выхода:																								//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::pinOutScheme		(uint8_t pin, uint8_t mode){											//	Параметры:				номер вывода, схема выхода.
		if((pin<8)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
			uint8_t min=0, max=7; if(pin<8){min=max=pin;}																//	Определяем минимальный и максимальный номер вывода.
		//	Читаем значение регистра «REG_EXP_OUT_MODE»:																//
			_readBytes(REG_EXP_OUT_MODE, 1);																			//	Читаем 1 байт из регистра «REG_EXP_OUT_MODE» в массив «data».
		//	Обновляем данные:																							//
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				if( mode  == OUT_OPEN_DRAIN  )	{ data[0] |=  (1<<i);	arrMode[i] |=  EXP_BIT_SCH; }					//	Если выход должен работать по схеме с открытым стоком, то устанавливаем бит регистра «REG_EXP_OUT_MODE» и устанавливаем бит «EXP_BIT_SCH» в массиве «arrMode».
				if( mode  == OUT_PUSH_PULL   )	{ data[0] &= ~(1<<i);	arrMode[i] &= ~EXP_BIT_SCH; }					//	Если выход должен работать по двухтактной схеме      , то сбрасываем    бит регистра «REG_EXP_OUT_MODE» и сбрасываем    бит «EXP_BIT_SCH» в массиве «arrMode».
			}																											//
		//	Записываем обновлённые данные в регистр «REG_EXP_OUT_MODE»:													//
			_writeBytes(REG_EXP_OUT_MODE, 1);																			//	Записываем 1 байт из массива «data» в регистр «REG_EXP_OUT_MODE».
		}																												//
		}																												//
}																														//
																														//
//		Установка логического уровня:																					//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::digitalWrite		(uint8_t pin, uint8_t level){											//	Параметры:				номер вывода, логический уровень.
		if((pin<8)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Проверяем конфигурацию вывода:																				//
			uint8_t f=0, min=0, max=7; if(pin<8){min=max=pin;}															//	Определяем флаг «f» указывающий на необходимость переконфигурирования вывода, а так же минимальный и максимальный номер вывода.
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				if((arrMode[i] & EXP_BIT_DIR) == 0){f=1; arrMode[i] |=  EXP_BIT_DIR;}									//	Если вывод «i» сконфигурирован как вход,       то устанавливаем флаг f и устанавливаем бит «EXP_BIT_DIR» в массиве «arrMode».
				if((arrMode[i] & EXP_BIT_TYP) >= 1){f=1; arrMode[i] &= ~EXP_BIT_TYP;}									//	Если вывод «i» сконфигурирован как аналоговый, то устанавливаем флаг f и сбрасываем    бит «EXP_BIT_TYP» в массиве «arrMode».
			}																											//
		//	Переконфигурируем вывод:																					//
			if(f){																										//	Если установлен флаг f, то ...
			//	Готовим два байта для записи:																			//
				data[0] = 0xFF;																							//	Устанавливаем все биты для регистра «REG_EXP_DIRECTION» (все выводы являются выходами  ).
				data[1] = 0x00;																							//	Сбрасываем    все биты для регистра «REG_EXP_TYPE»      (все выводы являются цифровыми ).
			//	Если указан один вывод:																					//
				if(pin<8){																								//	Если указан 1 вывод, то ...
				//	Читаем значение регистра «REG_EXP_DIRECTION» и следующего за ним регистра «REG_EXP_TYPE»:			//
					_readBytes(REG_EXP_DIRECTION, 2);																	//	Читаем 2 байта регистров «REG_EXP_DIRECTION» и «REG_EXP_TYPE» в массив «data».
					data[0] |=  (1<<pin);																				//	Устанавливаем бит для регистра «REG_EXP_DIRECTION» (вывод является выходом ).
					data[1] &= ~(1<<pin);																				//	Сбрасываем    бит для регистра «REG_EXP_TYPE»      (вывод является цифровым).
				}																										//
			//	Записываем обновлённые данные в регистр «REG_EXP_DIRECTION» и «REG_EXP_TYPE»:							//
				_writeBytes(REG_EXP_DIRECTION, 2);																		//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_DIRECTION».
			}																											//
		//	Если указан один вывод, то записываем бит в регистр «REG_EXP_WRITE_HIGH» или «REG_EXP_WRITE_LOW»:			//
			if(pin<8){																									//	Если указан 1 вывод, то ...
				data[0] = (1<<pin);																						//	Устанавливаем бит номер которого совпадает с номером вывода.
				if( level )	{ _writeBytes(REG_EXP_WRITE_HIGH, 1); }														//	Если устанавливается уровень логической «1», то записываем 1 байт данных из массива «data» в регистр «REG_EXP_WRITE_HIGH».
				else		{ _writeBytes(REG_EXP_WRITE_LOW,  1); }														//	Иначе, если устанавливается уровень лог «0», то записываем 1 байт данных из массива «data» в регистр «REG_EXP_WRITE_LOW».
		//	Если указаны все выводы, то записываем байт «level» в регистр «REG_EXP_DIGITAL»:							//
			}else{																										//	Если указаны все выводы, то ...
				data[0] = level;																						//	Меняем первый байт массива «data» на значение «level».
				_writeBytes(REG_EXP_DIGITAL, 1);																		//	Записываем 1 байт из массива «data» в регистр «REG_EXP_DIGITAL» модуля.
			}																											//
		}																												//
		}																												//
}																														//
																														//
//		Чтение логического уровня:																						//	Возвращаемое значение:	логический уровень.
uint8_t	iarduino_I2C_Expander::digitalRead		(uint8_t pin){															//	Параметр:				номер вывода.
		if((pin<8)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Проверяем конфигурацию вывода:																				//
			uint8_t f=0, min=0, max=7; if(pin<8){min=max=pin;}															//	Определяем флаг «f» указывающий на необходимость переконфигурирования вывода, а так же минимальный и максимальный номер вывода.
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				if((arrMode[i] & EXP_BIT_DIR) >= 1){f=1; arrMode[i] &= ~EXP_BIT_DIR;}									//	Если вывод «i» сконфигурирован как выход,      то устанавливаем флаг f и сбрасываем бит «EXP_BIT_DIR» в массиве «arrMode».
				if((arrMode[i] & EXP_BIT_TYP) >= 1){f=1; arrMode[i] &= ~EXP_BIT_TYP;}									//	Если вывод «i» сконфигурирован как аналоговый, то устанавливаем флаг f и сбрасываем бит «EXP_BIT_TYP» в массиве «arrMode».
			}																											//
		//	Переконфигурируем вывод:																					//
			if(f){																										//	Если установлен флаг f, то ...
			//	Готовим два байта для записи:																			//
				data[0] = 0x00;																							//	Сбрасываем все биты для регистра «REG_EXP_DIRECTION» (все выводы являются входами   ).
				data[1] = 0x00;																							//	Сбрасываем все биты для регистра «REG_EXP_TYPE»      (все выводы являются цифровыми ).
			//	Если указан один вывод:																					//
				if(pin<8){																								//	Если указан 1 вывод, то ...
				//	Читаем значение регистра «REG_EXP_DIRECTION» и следующего за ним регистра «REG_EXP_TYPE»:			//
					_readBytes(REG_EXP_DIRECTION, 2);																	//	Читаем 2 байта регистров    «REG_EXP_DIRECTION» и «REG_EXP_TYPE» в массив «data».
					data[0] &= ~(1<<pin);																				//	Сбрасываем бит для регистра «REG_EXP_DIRECTION» (вывод является входом  ).
					data[1] &= ~(1<<pin);																				//	Сбрасываем бит для регистра «REG_EXP_TYPE»      (вывод является цифровым).
				}																										//
			//	Записываем обновлённые данные в регистр «REG_EXP_DIRECTION» и «REG_EXP_TYPE»:							//
				_writeBytes(REG_EXP_DIRECTION, 2);																		//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_DIRECTION».
			}																											//
		//	Читаем логический уровень из регистра «REG_EXP_DIGITAL»:													//
			_readBytes(REG_EXP_DIGITAL, 1);																				//	Читаем 1 байт из регистра «REG_EXP_DIGITAL» в массив «data».
			if(pin<8)	{return (data[0] & (1<<pin))?1:0;}																//	Возвращаем значение бита под номером pin.
			else		{return  data[0];                }																//	Возвращаем 1 байт из массива «data».
		}																												//
		}																												//
		return 0;																										//	Возвращаем 0.
}																														//
																														//
//		Установка аналогового уровня:																					//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::analogWrite		(uint8_t pin, uint16_t level){											//	Параметры:				номер вывода, аналоговый уровень.
		if((pin<8)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Проверяем конфигурацию вывода:																				//
			uint8_t f=0, min=0, max=7; if(pin<8){min=max=pin;}															//	Определяем флаг «f» указывающий на необходимость переконфигурирования вывода, а так же минимальный и максимальный номер вывода.
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				if((arrMode[i] & EXP_BIT_DIR) == 0){f=1; arrMode[i] |= EXP_BIT_DIR;}									//	Если вывод «i» сконфигурирован как вход,     то устанавливаем флаг f и устанавливаем бит «EXP_BIT_DIR» в массиве «arrMode».
				if((arrMode[i] & EXP_BIT_TYP) == 0){f=1; arrMode[i] |= EXP_BIT_TYP;}									//	Если вывод «i» сконфигурирован как цифровой, то устанавливаем флаг f и устанавливаем бит «EXP_BIT_TYP» в массиве «arrMode».
			}																											//
		//	Переконфигурируем вывод:																					//
			if(f){																										//	Если установлен флаг f, то ...
			//	Читаем значение регистра «REG_EXP_DIRECTION» и следующего за ним регистра «REG_EXP_TYPE»:				//
				_readBytes(REG_EXP_DIRECTION, 2);																		//	Читаем 2 байта регистров «REG_EXP_DIRECTION» и «REG_EXP_TYPE» в массив «data».
			//	Обновляем данные:																						//
				for(uint8_t i=min; i<=max; i++){																		//	Проходим по одному или всем выводам.
					data[0] |= (1<<i);																					//	Устанавливаем бит для регистра «REG_EXP_DIRECTION» (вывод является выходом   ).
					data[1] |= (1<<i);																					//	Устанавливаем бит для регистра «REG_EXP_TYPE»      (вывод является аналоговым).
				}																										//
			//	Записываем обновлённые данные в регистр «REG_EXP_DIRECTION» и «REG_EXP_TYPE»:							//
				_writeBytes(REG_EXP_DIRECTION, 2);																		//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_DIRECTION».
			}																											//
		//	Записываем уровень «level» в регистры «REG_EXP_ANALOG»:														//
			for(uint8_t i=min, j=0; i<=max; i++, j++){																	//	Проходим по одному или всем выводам.
				data[j*2  ] =  level & 0xFF;																			//	Устанавливаем младший байт значения «level» для регистра «REG_EXP_ANALOG».
				data[j*2+1] = (level>>8);																				//	Устанавливаем старший байт значения «level» для регистра «REG_EXP_ANALOG».
			}																											//
			_writeBytes( REG_EXP_ANALOG + (min*2) , (max-min+1)*2 );													//	Записываем 2 или 16 байт из массива «data» в модуль начиная с регистра «REG_EXP_ANALOG».
		}																												//
		}																												//
}																														//
																														//
//		Чтение аналогового уровня:																						//	Возвращаемое значение:	аналоговый уровень.
uint16_t iarduino_I2C_Expander::analogRead		(uint8_t pin){															//	Параметр:				номер вывода.
		uint16_t level=0;																								//	Определяем переменную для вывода результата.
		if(pin<8){																										//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Проверяем конфигурацию вывода:																				//
			uint8_t f=0;																								//	Определяем флаг «f» указывающий на необходимость переконфигурирования вывода.
			if((arrMode[pin] & EXP_BIT_DIR) >= 1){f=1; arrMode[pin] &= ~EXP_BIT_DIR;}									//	Если вывод «i» сконфигурирован как выход,    то устанавливаем флаг f и сбрасываем    бит «EXP_BIT_DIR» в массиве «arrMode».
			if((arrMode[pin] & EXP_BIT_TYP) == 0){f=1; arrMode[pin] |=  EXP_BIT_TYP;}									//	Если вывод «i» сконфигурирован как цифровой, то устанавливаем флаг f и устанавливаем бит «EXP_BIT_TYP» в массиве «arrMode».
		//	Переконфигурируем вывод:																					//
			if(f){																										//	Если установлен флаг f, то ...
			//	Читаем значение регистра «REG_EXP_DIRECTION» и следующего за ним регистра «REG_EXP_TYPE»:				//
				_readBytes(REG_EXP_DIRECTION, 2);																		//	Читаем 2 байта регистров «REG_EXP_DIRECTION» и «REG_EXP_TYPE» в массив «data».
			//	Обновляем данные:																						//
				data[0] &= ~(1<<pin);																					//	Сбрасываем    бит для регистра «REG_EXP_DIRECTION» (вывод является входом    ).
				data[1] |=  (1<<pin);																					//	Устанавливаем бит для регистра «REG_EXP_TYPE»      (вывод является аналоговым).
			//	Записываем обновлённые данные в регистр «REG_EXP_DIRECTION» и «REG_EXP_TYPE»:							//
				_writeBytes(REG_EXP_DIRECTION, 2);																		//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_DIRECTION».
			}																											//
		//	Читаем уровень из регистров «REG_EXP_ANALOG»:																//
			_readBytes( REG_EXP_ANALOG + (pin*2) , 2 );																	//	Читаем 2 байта из регистров «REG_EXP_ANALOG» в массив «data».
			level=data[1]; level<<=8; level|=data[0];																	//	Сохраняем 2 прочитанных байта в переменную «level».
		}																												//
		}																												//
		return level;																									//	Возвращаем результат чтения.
}																														//
																														//
//		Установка коэффициента усреднения показаний АЦП:																//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::analogAveraging	(uint8_t coefficient){													//	Параметр:				коэффициент усреднения от 0 до 255.
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
			data[0] = coefficient;																						//	Копируем значение аргумента «coefficient» в первый элемент массива «data».
			_writeBytes(REG_EXP_AVERAGING, 1);																			//	Записываем 1 байт из массива «data» в регистр «REG_EXP_AVERAGING» модуля.
		}																												//
}																														//
																														//
//		Установка аналогового уровня для функции levelRead():															//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::levelWrite		(uint16_t level){														//	Параметр:				аналоговый уровень.
		if(level<4095){																									//	Если корректно указан аналоговый уровень, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Готовим два байта для записи:																				//
			data[0] =  level & 0xFF;																					//	Устанавливаем младший байт значения «level» для регистра «REG_EXP_LEVEL_L».
			data[1] = (level>>8);																						//	Устанавливаем старший байт значения «level» для регистра «REG_EXP_LEVEL_H».
		//	Записываем обновлённые данные в регистр «REG_EXP_LEVEL_H» и «REG_EXP_LEVEL_L»:								//
			_writeBytes(REG_EXP_LEVEL_L, 2);																			//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_LEVEL_L».
		}																												//
		}																												//
}																														//
																														//
//		Чтение логичекого уровня с аналогового вывода:																	//	Возвращаемое значение:	логический уровень.
uint8_t	iarduino_I2C_Expander::levelRead		(uint8_t pin){															//	Параметр:				номер вывода.
		if((pin<8)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Проверяем конфигурацию вывода:																				//
			uint8_t f=0, min=0, max=7; if(pin<8){min=max=pin;}															//	Определяем флаг «f» указывающий на необходимость переконфигурирования вывода, а так же минимальный и максимальный номер вывода.
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				if((arrMode[i] & EXP_BIT_DIR) >= 1){f=1; arrMode[i] &= ~EXP_BIT_DIR;}									//	Если вывод «i» сконфигурирован как выход,    то устанавливаем флаг f и сбрасываем    бит «EXP_BIT_DIR» в массиве «arrMode».
				if((arrMode[i] & EXP_BIT_TYP) == 0){f=1; arrMode[i] |=  EXP_BIT_TYP;}									//	Если вывод «i» сконфигурирован как цифровой, то устанавливаем флаг f и устанавливаем бит «EXP_BIT_TYP» в массиве «arrMode».
			}																											//
		//	Переконфигурируем вывод:																					//
			if(f){																										//	Если установлен флаг f, то ...
			//	Готовим два байта для записи:																			//
				data[0] = 0x00;																							//	Сбрасываем    все биты для регистра «REG_EXP_DIRECTION» (все выводы являются входами     ).
				data[1] = 0xFF;																							//	Устанавливаем все биты для регистра «REG_EXP_TYPE»      (все выводы являются аналоговыми ).
			//	Если указан один вывод:																					//
				if(pin<8){																								//	Если указан 1 вывод, то ...
				//	Читаем значение регистра «REG_EXP_DIRECTION» и следующего за ним регистра «REG_EXP_TYPE»:			//
					_readBytes(REG_EXP_DIRECTION, 2);																	//	Читаем 2 байта регистров «REG_EXP_DIRECTION» и «REG_EXP_TYPE» в массив «data».
					data[0] &= ~(1<<pin);																				//	Сбрасываем    бит для регистра «REG_EXP_DIRECTION» (вывод является входом    ).
					data[1] |=  (1<<pin);																				//	Устанавливаем бит для регистра «REG_EXP_TYPE»      (вывод является аналоговым).
				}																										//
			//	Записываем обновлённые данные в регистр «REG_EXP_DIRECTION» и «REG_EXP_TYPE»:							//
				_writeBytes(REG_EXP_DIRECTION, 2);																		//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_DIRECTION».
			}																											//
		//	Читаем логический уровень из регистра «REG_EXP_DIGITAL»:													//
			_readBytes(REG_EXP_DIGITAL, 1);																				//	Читаем 1 байт из регистра «REG_EXP_DIGITAL» в массив «data».
			if(pin<8)	{return (data[0] & (1<<pin))?1:0;}																//	Возвращаем значение бита под номером pin.
			else		{return  data[0];                }																//	Возвращаем 1 байт из массива «data».
		}																												//
		}																												//
		return 0;																										//	Возвращаем 0.
}																														//
																														//
//		Установка гистерезиса для функции levelRead():																	//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::levelHyst		(uint16_t hysteresis){													//	Параметр:				гистерезис.
		if(hysteresis<4095){																							//	Если корректно указан гистерезис, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Готовим два байта для записи:																				//
			data[0] =  hysteresis & 0xFF;																				//	Устанавливаем младший байт значения «hysteresis» для регистра «REG_EXP_HYSTERESIS_L».
			data[1] = (hysteresis>>8);																					//	Устанавливаем старший байт значения «hysteresis» для регистра «REG_EXP_HYSTERESIS_H».
		//	Записываем обновлённые данные в регистр «REG_EXP_HYSTERESIS_H» и «REG_EXP_HYSTERESIS_L»:					//
			_writeBytes(REG_EXP_HYSTERESIS_L, 2);																		//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_HYSTERESIS_L».
		}																												//
		}																												//
}																														//
																														//
//		Установка частоты ШИМ:																							//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::freqPWM			(uint16_t frequency){													//	Параметр:				частота в Гц.
		if((frequency>0)&&(frequency<=12000)){																			//	Если корректно указана частота, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
		//	Готовим два байта для записи:																				//
			data[0]		=  frequency & 0xFF;																			//	Устанавливаем младший байт значения «frequency» для регистра «REG_EXP_FREQUENCY_L».
			data[1]		= (frequency>>8);																				//	Устанавливаем старший байт значения «frequency» для регистра «REG_EXP_FREQUENCY_H».
			valFreqPWM	=  frequency;																					//	Сохраняем частоту ШИМ для её сравнения при управлении сервоприводами.
		//	Записываем обновлённые данные в регистр «REG_EXP_FREQUENCY_H» и «REG_EXP_FREQUENCY_L»:						//
			_writeBytes(REG_EXP_FREQUENCY_L, 2);																		//	Записываем 2 байта из массива «data» в модуль, начиная с регистра «REG_EXP_FREQUENCY_L».
		}																												//
		}																												//
}																														//
																														//
//		Конфигурирование вывода для сервопривода:																		//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::servoAttach(uint8_t pin,uint16_t width_min,uint16_t width_max,int16_t angle_min,int16_t angle_max){	//		номер вывода, минимальная ширина импульса, максимальная ширина импульса, [минимальный угол, максимальный угол].
		if((pin<4)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
			uint8_t min=0, max=3; if(pin<4){min=max=pin;}																//	Определяем минимальный и максимальный номер вывода.
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				arrServ[i][0] = (int16_t) width_min;																	//	Сохраняем минимальную  ширину импульса в мкс.
				arrServ[i][1] = (int16_t) width_max;																	//	Сохраняем максимальную ширину импульса в мкс.
				arrServ[i][2] = angle_min;																				//	Сохраняем минимальный  угол поворота сервопривода в градусах.
				arrServ[i][3] = angle_max;																				//	Сохраняем максимальный угол поворота сервопривода в градусах.
			}																											//
		}																												//
		}																												//
}																														//
																														//
//		Установка угла поворота сервопривода:																			//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::servoWrite		(uint8_t pin, int16_t angle){											//	Параметры:				номер вывода, угол поворота.
		if((pin<4)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
			uint8_t min=0, max=3; if(pin<4){min=max=pin;}																//	Определяем минимальный и максимальный номер вывода.
			for(uint8_t i=min; i<=max; i++){																			//	Проходим по одному или всем выводам.
				servoWriteMicroseconds(i, map(angle,arrServ[i][2],arrServ[i][3],arrServ[i][0],arrServ[i][1]));			//	Устанавшиваем ШИМ в мкс, преобразовав угол «angle» от диапазона arrServ[pin][2]°...arrServ[pin][3]° к диапазону arrServ[pin][0]мкс...arrServ[pin][1]мкс.
			}																											//
		}																												//
		}																												//
}																														//
																														//
//		Установка длительности импульсов для сервопривода:																//	Возвращаемое значение:	отсутствует.
void	iarduino_I2C_Expander::servoWriteMicroseconds(uint8_t pin, uint16_t width){										//	Параметры:				номер вывода, длительность импульсов.
		if((pin<4)||(pin==ALL_PIN)){																					//	Если корректно указан номер вывода, то ...
		if(valAddr){																									//	Если расширитель был инициализирован, то ...
			if(valFreqPWM != 50){ freqPWM(50); }																		//	Устанавливаем частоту ШИМ = 50 Гц.
			analogWrite(pin, uint16_t((double)width*0.20475));															//	Устанавливаем ШИМ с коэффициентом заполнения = width * 4095 / 20000 мкс, где 20000 мкс это период частоты в 50 Гц.
		}																												//
		}																												//
}																														//
																														//
//		Чтение данных из регистров в массив data:																		//	Возвращаемое значение:	результат чтения (true/false).
bool	iarduino_I2C_Expander::_readBytes		(uint8_t reg, uint8_t sum){												//	Параметры:				reg - номер первого регистра, sum - количество читаемых байт.
			bool	result=false;																						//	Определяем флаг       для хранения результата чтения.
			uint8_t	sumtry=10;																							//	Определяем переменную для подсчёта количества оставшихся попыток чтения.
			do{	result = objI2C->readBytes(valAddr, reg, data, sum);													//	Считываем из модуля valAddr, начиная с регистра reg, в массив data, sum байт.
				sumtry--;	if(!result){delay(1);}																		//	Уменьшаем количество попыток чтения и устанавливаем задержку при неудаче.
			}	while		(!result && sumtry>0);																		//	Повторяем чтение если оно завершилось неудачей, но не более sumtry попыток.
			return result;																								//	Возвращаем результат чтения (true/false).
}																														//
																														//
//		Запись данных в регистры из массива data:																		//	Возвращаемое значение:	результат записи (true/false).
bool	iarduino_I2C_Expander::_writeBytes	(uint8_t reg, uint8_t sum, uint8_t num){									//	Параметры:				reg - номер первого регистра, sum - количество записываемых байт, num - номер первого элемента массива data.
			bool	result=false;																						//	Определяем флаг       для хранения результата записи.
			uint8_t	sumtry=10;																							//	Определяем переменную для подсчёта количества оставшихся попыток записи.
			do{	result = objI2C->writeBytes(valAddr, reg, &data[num], sum);												//	Записываем в модуль valAddr начиная с регистра reg, sum байи из массива data начиная с элемента num.
				sumtry--;	if(!result){delay(1);}																		//	Уменьшаем количество попыток записи и устанавливаем задержку при неудаче.
			}	while		(!result && sumtry>0);																		//	Повторяем запись если она завершилась неудачей, но не более sumtry попыток.
			delay(2);																									//	Ждём применения модулем записанных данных.
			return result;																								//	Возвращаем результат записи (true/false).
}																														//
																														//