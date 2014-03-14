package code
{
	import flash.display.LoaderInfo;
	import flash.utils.ByteArray;
	import flash.utils.Endian;
	
	public class BuildTimeStamp
	{
		public function BuildTimeStamp()
		{
		}
		
		///////////////////////////////////////////////////////////////////////////
		// Returns compilation date of current module
		public static function readCompilationDate(li:LoaderInfo, serialNumber: ByteArray = null): Date
		{
			const compilationDate: Date = new Date;
			
			var o:Object = readSerialNumber(li);
			
			if (serialNumber == null)
				serialNumber = o.tagBody;
			
			trace("Len: " + o.tagLength);
			
			/* example of filled SWF_SERIALNUMBER structure
			struct SWF_SERIALNUMBER
			{
			UI32 Id;         // "3"
			UI32 Edition;    // "6"
			// "flex_sdk_4.0.0.3342"
			UI8 Major;       // "4."
			UI8 Minor;       // "0."
			UI32 BuildL;     // "0."
			UI32 BuildH;     // "3342"
			UI32 TimestampL;
			UI32 TimestampH;
			};
			*/
			
			// the SWF_SERIALNUMBER structure exists in FLEX swfs only, not FLASH
			if (serialNumber == null)
				return null;
			
			// date stored as uint64
			
			serialNumber.endian = Endian.LITTLE_ENDIAN;
			trace("BuildHi:" + serialNumber.readUnsignedInt());
			trace("BuildLo:" + serialNumber.readUnsignedInt());
			trace("Edition:" + serialNumber.readUnsignedInt());
			trace("ID:" + serialNumber.readUnsignedInt());
			trace("Major:" + serialNumber.readUnsignedByte());
			trace("Minor:" + serialNumber.readUnsignedByte());
			var l:uint = serialNumber.readUnsignedInt();
			var h:uint = serialNumber.readUnsignedInt();
			trace("TSLO:" + l);
			trace("TSHI:" + h);
			while(serialNumber.bytesAvailable)
			{
				trace(serialNumber.readUnsignedByte().toString(16));
			}

			compilationDate.time = l + h * (uint.MAX_VALUE + 1);
			
			return compilationDate;
		}
		
		///////////////////////////////////////////////////////////////////////////
		// Returns contents of Adobe SerialNumber SWF tag
		public static function readSerialNumber(li:LoaderInfo): Object
		{
			const TAG_SERIAL_NUMBER: uint = 0x29;
			return findAndReadTagBody(li, TAG_SERIAL_NUMBER);
		}
		
		///////////////////////////////////////////////////////////////////////////
		// Returns the tag body if it is possible
		public static function findAndReadTagBody(li:LoaderInfo, theTagCode: uint): Object
		{
			// getting direst access to unpacked SWF file
			const src: ByteArray = li.bytes;
			
			/*
			SWF File Header
			Field      Type  Offset   Comment
			-----      ----  ------   -------
			Signature  UI8   0        Signature byte: “F” indicates uncompressed, “C” indicates compressed (SWF 6 and later only)
			Signature  UI8   1        Signature byte always “W”
			Signature  UI8   2        Signature byte always “S”
			Version    UI8   3        Single byte file version (for example, 0x06 for SWF 6)
			FileLength UI32  4        Length of entire file in bytes
			FrameSize  RECT  8        Frame size in twips
			FrameRate  UI16  8+RECT   Frame delay in 8.8 fixed number of frames per second
			FrameCount UI16  10+RECT  Total number of frames in file
			*/
			
			// skip AVM2 SWF header
			// skip Signature, Version & FileLength
			src.position = 8;
			// skip FrameSize
			const RECT_UB_LENGTH: uint = 5;
			const RECT_SB_LENGTH: uint = src.readUnsignedByte() >> (8 - RECT_UB_LENGTH);
			const RECT_LENGTH: uint = Math.ceil((RECT_UB_LENGTH + RECT_SB_LENGTH * 4) / 8);
			src.position += (RECT_LENGTH - 1);
			// skip FrameRate & FrameCount
			src.position += 4;
			
			while (src.bytesAvailable > 0)
				with (readTag(src, theTagCode))
				{
					if (tagCode == theTagCode)
						return { tagBody: tagBody, tagLength:tagLength };
				}
			
			return null;
		}
		
		///////////////////////////////////////////////////////////////////////////
		// Returns tag from current read position
		private static function readTag(src: ByteArray, theTagCode: uint): Object
		{
			src.endian = Endian.LITTLE_ENDIAN;
			
			const tagCodeAndLength: uint = src.readUnsignedShort();
			const tagCode: uint = tagCodeAndLength >> 6;
			const tagLength: uint = function(): uint {
				const MAX_SHORT_TAG_LENGTH: uint = 0x3F;
				const shortLength: uint = tagCodeAndLength & MAX_SHORT_TAG_LENGTH;
				return (shortLength == MAX_SHORT_TAG_LENGTH) ? src.readUnsignedInt() : shortLength;
			}();
			
			const tagBody: ByteArray = new ByteArray;
			if (tagLength > 0)
				src.readBytes(tagBody, 0, tagLength);
			
			return {
				tagCode: tagCode,
				tagBody: tagBody,
				tagLength: tagLength
			};
		}
	}
}
