package code
{
	//////////////////////////////////////////////////////////////////////

	import flash.utils.ByteArray;
	import flash.utils.Endian;
	
	import br.com.stimuli.loading.BulkLoader;

	//////////////////////////////////////////////////////////////////////

	public class Dictionary
	{
		//////////////////////////////////////////////////////////////////////

		public static var sWords: Array;
		
		private static var sFile : ByteArray;
		private static var sDefinitions : Array;

		private static const kNone						:uint =	0;
		private static const kPastTense					:uint =	1;
		private static const kPlural					:uint =	2;
		private static const kComparative				:uint =	3;
		private static const kSuperlative				:uint =	4;
		private static const kObsoleteForm				:uint =	5;
		private static const kObsoleteSpelling			:uint =	6;
		private static const kAltCapitalization			:uint =	7;
		private static const kAltSpelling				:uint =	8;
		private static const kInformalSpelling			:uint =	9;
		private static const kArchaicSpelling			:uint = 10;
		private static const kArchaicForm				:uint = 11;
		private static const kArchaicThirdPersonSingular:uint = 12;
		private static const kPresentTense				:uint = 13;
		private static const kThirdPersonSingular		:uint = 14;
		private static const kSecondPersonSingular		:uint = 15;
		private static const kMisspelling				:uint = 16;
		private static const kAltForm					:uint = 17;
		private static const kDatedForm					:uint = 18;
		private static const kObsoleteTypography		:uint = 19;
		private static const kEyeDialect				:uint = 20;
		private static const kAbbreviation				:uint = 21;
		private static const kNumReferenceTypes			:uint = 22;
		
		private static const kNoun						:uint =  0;
		private static const kVerb						:uint =  1;
		private static const kAdjective					:uint =  2;
		private static const kAdverb					:uint =  3;
		private static const kConjunction				:uint =  4;
		private static const kPronoun					:uint =  5;
		private static const kPreposition				:uint =  6;
		private static const kNumeral					:uint =  7;
		private static const kNumWordTypes				:uint =  8;

		private static const sReferenceText:Array =
		[
			"",
			"Past tense",
			"Plural",
			"Comparative",
			"Superlative",
			"Obsolete form",
			"Obsolete spelling",
			"Alt. capitalization",
			"Alt. spelling",
			"Informal spelling",
			"Archaic spelling",
			"Archaic form",
			"Archaic 3rd person singular",
			"Present tense",
			"3rd person singular",
			"2nd person singular",
			"Misspelling",
			"Alt. form",
			"Dated form",
			"Obsolete typography",
			"Eye dialect",
			"Abbreviation",
		];
		
		private static const sDefinitionNames:Array =
		[
			"n",
			"v",
			"adj",
			"adv",
			"conj",
			"pron",
			"prep",
			"numeral"
		];
		
		//////////////////////////////////////////////////////////////////////
		
		public static function Preload():void
		{
			Globals.AddLoadItem("english.dictionary", true, BulkLoader.TYPE_BINARY);
		}

		//////////////////////////////////////////////////////////////////////
		
		public static function Load():void
		{
			sFile = Globals.Loader.getBinary(Globals.URL("english.dictionary"));
			sFile.endian = Endian.LITTLE_ENDIAN;
			
			var header:ByteArray = new ByteArray();
			sFile.readBytes(header, 0, 10);
			if (header.toString() == "dictionary")
			{
				sWords = new Array();
				var version:uint = sFile.readUnsignedShort();
				var wordCount:uint = sFile.readUnsignedInt();
				
				//trace("Version: " + version);
				//trace("WordCount: " + wordCount);
				
				for (var i:uint=0; i<wordCount; ++i)
				{
					sWords.push(sFile.readUTFBytes(8));
				}
				
				sDefinitions = new Array(wordCount);
				var currentWord:uint = 0;

				while(sFile.bytesAvailable)
				{
					sDefinitions[currentWord] = sFile.position;
					
					var mask:uint = sFile.readUnsignedShort();
					
					for(i=0; i<kNumWordTypes; ++i)
					{
						if((mask & (1<<i)) != 0)
						{
							var w:uint = sFile.readUnsignedByte();
							if(w == kNone)
							{
								ReadString();
							}
							else
							{
								sFile.position += 3;
							}
						}
					}
					++currentWord;
				}
				
			}
			else
			{
				trace("Not a dictionary!");
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		public static function GetDefinition(word:uint):String
		{
			var definition:String = new String();			
			if(word < sWords.length)
			{
				var offset:uint = sDefinitions[word];
				sFile.position = offset;
				var mask:uint = sFile.readUnsignedShort();
				for(var i:uint=0; i<kNumWordTypes; ++i)
				{
					if((mask & (1 << i)) != 0)
					{
						definition += " (";
						definition += sDefinitionNames[i];
						definition += ") ";
						
						var type:uint = sFile.readByte();
						if(type == kNone)
						{
							definition += ReadString();
						}
						else
						{
							var w:uint = ReadU24();
							definition += sReferenceText[type];
							definition += " of @";
							definition += sWords[w];
							definition += "@";
						}
						definition += "\n";
					}
				}
			}
			return definition;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public static function WordIndex(word:String):int
		{
			var t:int = sWords.length - 1;
			var b:int = 0;
			while (b <= t)
			{
				var m:int = b + (t - b) / 2;
				var c:int = word.localeCompare(sWords[m]);

				if (c > 0)
				{
					b = m + 1;
				}
				else if (c < 0)
				{
					t = m - 1;
				}
				else
				{
					return m;
				}
			}
			return -1;
		}

		//////////////////////////////////////////////////////////////////////
		
		private static function ReadU24():uint
		{
			var r:uint = 0;
			for(var i:uint=0; i<3; ++i)
			{
				r |= sFile.readUnsignedByte() << (i * 8);
			}
			return r;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private static function ReadString():String
		{
			var p:uint = sFile.position;
			var l:uint = 0;
			while(sFile[p + l] != 0)
			{
				++l;
			}
			return sFile.readUTFBytes(l + 1);
		}
	}
}