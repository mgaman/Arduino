use WaterMeter;
#insert into rawdata (action,imei,ExternalMeter) values('calibrate','123',830.9756);
#select * from rawdata where action='calibrate' order by serverts desc;
#select * from rawdata  order by serverts desc;
#select * from SoilWater order by TS desc;
#select tpl0,tpl1,TS from EEPROM order by TS desc limit 10;
#insert into EEPROM (tpl0,tpl1) values(236,215);
#select ID,TS,ADC from SoilWater where TIMESTAMPDIFF(SECOND,TS,CURRENT_TIMESTAMP) < 7200 and ID=99 order by TS desc;
select * from SoilWater where ID=98;
#select * from rawdata order by serverts desc;
