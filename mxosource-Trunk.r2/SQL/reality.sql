SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;


CREATE TABLE IF NOT EXISTS `characters` (
  `charId` bigint(30) unsigned NOT NULL,
  `userId` int(11) unsigned NOT NULL,
  `worldId` smallint(5) unsigned NOT NULL DEFAULT '1',
  `status` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'transit/banned',
  `handle` varchar(32) NOT NULL,
  `firstName` varchar(32) NOT NULL,
  `lastName` varchar(32) NOT NULL,
  `background` varchar(1024) DEFAULT NULL,
  `x` double NOT NULL DEFAULT '12223.2',
  `y` double NOT NULL DEFAULT '-705',
  `z` double NOT NULL DEFAULT '59707.3',
  `rot` double NOT NULL DEFAULT '0',
  `healthC` mediumint(11) NOT NULL DEFAULT '1000',
  `healthM` mediumint(11) NOT NULL DEFAULT '1000',
  `innerStrC` mediumint(11) NOT NULL DEFAULT '50',
  `innerStrM` mediumint(11) NOT NULL DEFAULT '50',
  `level` mediumint(11) NOT NULL DEFAULT '1',
  `profession` int(10) NOT NULL DEFAULT '2',
  `alignment` smallint(6) NOT NULL DEFAULT '50',
  `pvpflag` smallint(6) NOT NULL DEFAULT '0',
  `exp` bigint(30) NOT NULL DEFAULT '0',
  `cash` bigint(30) NOT NULL DEFAULT '500',
  `district` int(11) unsigned NOT NULL DEFAULT '1',
  `adminFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `lastOnline` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `isOnline` tinyint(2) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`charId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `doors` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `doorid` int(11) NOT NULL,
  `districtid` int(11) NOT NULL,
  `X` double NOT NULL DEFAULT '0',
  `Y` double NOT NULL DEFAULT '0',
  `Z` double NOT NULL DEFAULT '0',
  `ROT` double NOT NULL DEFAULT '0',
  `Open` bit(1) NOT NULL DEFAULT b'0',
  `OpenTime` datetime DEFAULT NULL,
  `DoorType` int(3) NOT NULL DEFAULT '1',
  `FirstUser` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS `hardlines` (
  `Id` int(11) unsigned NOT NULL,
  `HardLineId` smallint(6) NOT NULL,
  `HardlineName` varchar(45) NOT NULL,
  `X` double NOT NULL,
  `Y` double NOT NULL,
  `Z` double NOT NULL,
  `ROT` double NOT NULL,
  `DistrictId` smallint(6) NOT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `inventory` (
  `invId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `charId` bigint(30) unsigned NOT NULL,
  `goid` int(11) unsigned NOT NULL,
  `slot` tinyint(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`invId`),
  UNIQUE KEY `invId` (`invId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS `locations` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Command` varchar(45) NOT NULL,
  `X` double NOT NULL,
  `Y` double NOT NULL,
  `Z` double NOT NULL,
  `District` tinyint(3) NOT NULL,
  PRIMARY KEY (`Id`,`Command`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

CREATE TABLE IF NOT EXISTS `myappearence` (
  `handle` varchar(45) NOT NULL,
  `hat` varchar(9) NOT NULL DEFAULT '00000000',
  `glasses` varchar(9) NOT NULL DEFAULT '00000000',
  `shirt` varchar(9) NOT NULL DEFAULT '00000000',
  `gloves` varchar(9) NOT NULL DEFAULT '00000000',
  `coat` varchar(9) NOT NULL DEFAULT '00000000',
  `pants` varchar(9) NOT NULL DEFAULT '00000000',
  `shoes` varchar(9) NOT NULL DEFAULT '00000000',
  `weapon` varchar(9) NOT NULL DEFAULT '00000000',
  `tool` varchar(9) NOT NULL DEFAULT '00000000',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myattributes` (
  `handle` varchar(45) NOT NULL,
  `belief` int(11) unsigned NOT NULL,
  `percepition` int(11) unsigned NOT NULL,
  `reason` int(11) unsigned NOT NULL,
  `focus` int(11) unsigned NOT NULL,
  `vitality` int(11) unsigned NOT NULL,
  `total` int(11) unsigned NOT NULL,
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `mybuddylist` (
  `handle` varchar(45) NOT NULL,
  `buddyList` varchar(10000) NOT NULL DEFAULT 'NONE',
  `limit` int(11) NOT NULL DEFAULT '50',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myclothes` (
  `clothNumber` int(11) NOT NULL AUTO_INCREMENT,
  `clothGender` varchar(20) NOT NULL,
  `clothType` varchar(20) NOT NULL,
  `clothId` varchar(9) NOT NULL,
  `clothName` varchar(256) NOT NULL,
  `model` int(11) NOT NULL DEFAULT '0',
  `color` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`clothNumber`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=2430 ;

CREATE TABLE IF NOT EXISTS `mycrew` (
  `handle` varchar(45) NOT NULL,
  `IN_CREW` int(11) NOT NULL DEFAULT '0',
  `COMMAND` varchar(45) NOT NULL DEFAULT 'NONE',
  `SLOTS` int(11) NOT NULL DEFAULT '50',
  `MEMBERS` varchar(15000) NOT NULL DEFAULT 'NONE',
  `NAME_CREW` varchar(45) NOT NULL DEFAULT 'NONE',
  `CAPTAIN` varchar(45) NOT NULL DEFAULT 'NONE',
  `STATE` varchar(45) NOT NULL DEFAULT 'NONE',
  `ID_CREW` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myemailaccount` (
  `handle` varchar(45) NOT NULL,
  `emails` varchar(863) DEFAULT NULL,
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myemaildb` (
  `code` varchar(45) NOT NULL,
  `date` varchar(45) NOT NULL,
  `sender` varchar(45) NOT NULL,
  `subject` varchar(45) NOT NULL,
  `message` varchar(2500) NOT NULL,
  PRIMARY KEY (`code`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myinstallation` (
  `userId` int(11) NOT NULL,
  `userName` varchar(30) NOT NULL,
  `isCreatingCharacter` int(11) NOT NULL DEFAULT '0' COMMENT '1=YES, 0=NO',
  `handle` varchar(30) NOT NULL,
  `gender` int(11) NOT NULL DEFAULT '0' COMMENT '1=FM, 0=M',
  PRIMARY KEY (`userId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myinventory` (
  `handle` varchar(45) NOT NULL,
  `inventory` varchar(863) DEFAULT NULL,
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myitems` (
  `itemNumber` int(11) NOT NULL DEFAULT '0',
  `itemType` varchar(45) NOT NULL COMMENT 'SKILL, CLOTH, HAT_BANDANA, PANTS, SHIRT, SHOES, UNKNOWN, MISSION_QUEST, ITEM_PROP, ITEM_RSI_CAPTURE, ITEM_RSI_DISGUISE, KEY, NPC_PART, WEAPON',
  `itemId` varchar(45) NOT NULL,
  `itemName` varchar(45) NOT NULL,
  `cash` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemNumber`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myloot` (
  `handle` varchar(45) NOT NULL,
  `loot` varchar(20000) NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `mymarketplace` (
  `handle` varchar(45) NOT NULL,
  `market` varchar(5183) DEFAULT NULL,
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `mymissions` (
  `MISSION_ID` int(11) NOT NULL AUTO_INCREMENT,
  `MISSION_NAME` varchar(45) NOT NULL DEFAULT '0',
  `MISSION_DATA_ZI` varchar(15000) NOT NULL DEFAULT 'NONE',
  `MISSION_DATA_ME` varchar(15000) NOT NULL DEFAULT 'NONE',
  `MISSION_DATA_MA` varchar(15000) NOT NULL DEFAULT 'NONE',
  `DISTRICT` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`MISSION_ID`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

CREATE TABLE IF NOT EXISTS `mynewskills` (
  `skillId` varchar(9) NOT NULL DEFAULT '0000',
  `skillName` varchar(45) NOT NULL DEFAULT 'NONE',
  `animForMe` varchar(10) NOT NULL DEFAULT '000000',
  `animForMePVE` varchar(10) NOT NULL DEFAULT '000000',
  `animForOpponentPVP` varchar(10) NOT NULL DEFAULT '000000',
  `animForOpponentPVE` varchar(10) NOT NULL DEFAULT '000000',
  `CAST_FX` varchar(10) NOT NULL DEFAULT 'FALSE',
  `FXForMe` varchar(10) NOT NULL DEFAULT '00000000',
  `FXForOpponentPVP` varchar(10) NOT NULL DEFAULT '00000000',
  `DAMAGE` int(11) NOT NULL DEFAULT '0',
  `DISTANCE` int(11) NOT NULL DEFAULT '0',
  `weaponForMe` varchar(5) NOT NULL DEFAULT '0000',
  `REQUIRED_STATE` varchar(45) NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`skillId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `mynpcs` (
  `npcNumber` int(11) NOT NULL,
  `district` int(11) NOT NULL,
  `npcType` varchar(5) NOT NULL DEFAULT 'R' COMMENT 'R=Hostile, B=Friendly, Y=Contact',
  `handle` varchar(30) NOT NULL DEFAULT 'NONE',
  `viewId` varchar(5) NOT NULL DEFAULT '0000',
  `npcRSI` varchar(10) NOT NULL DEFAULT '00000000',
  `X_POS` int(11) NOT NULL DEFAULT '0',
  `Y_POS` int(11) NOT NULL DEFAULT '0',
  `Z_POS` int(11) NOT NULL DEFAULT '0',
  `TARGET_X_POS` int(11) NOT NULL DEFAULT '0',
  `TARGET_Y_POS` int(11) NOT NULL DEFAULT '0',
  `TARGET_Z_POS` int(11) NOT NULL DEFAULT '0',
  `mood` varchar(3) NOT NULL DEFAULT '00',
  `priority` varchar(3) NOT NULL DEFAULT '00' COMMENT '01=x3, 02=x2, 03x1, 04=x0',
  `lvl` int(11) NOT NULL DEFAULT '0',
  `currHP` int(11) NOT NULL DEFAULT '0',
  `maxHP` int(11) NOT NULL DEFAULT '0',
  `fx` varchar(10) NOT NULL DEFAULT '00000000',
  `buffer` int(11) NOT NULL DEFAULT '0',
  `STATE` varchar(30) NOT NULL DEFAULT 'NONE' COMMENT 'WALKING, ROTATING, NONE(STANDING)',
  PRIMARY KEY (`npcNumber`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myplayers` (
  `handle` varchar(45) NOT NULL,
  `fx` varchar(45) NOT NULL DEFAULT '00000000',
  `fx2` varchar(45) NOT NULL DEFAULT '00000000',
  `PAL` int(11) NOT NULL DEFAULT '0' COMMENT '0=PLAYER, 1=ADMINLESIG',
  `RSI` varchar(45) NOT NULL DEFAULT '00000000' COMMENT 'Reference: MYRSIMASK-RSI_NAME',
  `hp` int(11) NOT NULL DEFAULT '0',
  `update` varchar(45) NOT NULL DEFAULT 'NONE',
  `cash` varchar(45) NOT NULL DEFAULT 'NONE',
  `MASSIVE_CHAT` varchar(45) NOT NULL DEFAULT '0',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myquests` (
  `handle` varchar(45) NOT NULL,
  `MISSION_STATE` int(11) NOT NULL DEFAULT '0',
  `MISSION_ID` int(11) NOT NULL DEFAULT '0',
  `TOTAL_OBJECTIVES` int(11) NOT NULL DEFAULT '0',
  `MISSION_OBJECTIVE` int(11) NOT NULL DEFAULT '0',
  `MISSION_ORGANIZATION` varchar(45) NOT NULL DEFAULT 'NONE',
  `MISSION_DATA` varchar(15000) NOT NULL DEFAULT 'NONE',
  `NPC_FOR_ME` varchar(45) NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myreputation` (
  `handle` varchar(45) NOT NULL,
  `ZION` int(11) NOT NULL DEFAULT '0',
  `MACHINIST` int(11) NOT NULL DEFAULT '0',
  `MEROVINGIAN` int(11) NOT NULL DEFAULT '0',
  `EPN` int(11) NOT NULL DEFAULT '0',
  `CYPH` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myrsimask` (
  `rsiName` varchar(45) NOT NULL DEFAULT 'UNKNOWN',
  `rsiHexPacket` varchar(10) NOT NULL DEFAULT '00000000',
  `rsiHexInventory` varchar(10) NOT NULL DEFAULT '00000000' COMMENT 'Reference: MYITEMS-ITEM_RSI_CAPTUREDISGUISE',
  PRIMARY KEY (`rsiName`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myrsipills` (
  `pillNumber` int(11) NOT NULL,
  `param` varchar(45) NOT NULL,
  `pillId` varchar(9) NOT NULL,
  `pillName` varchar(45) NOT NULL,
  `value` varchar(2) NOT NULL,
  PRIMARY KEY (`pillId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myskills` (
  `skillId` varchar(9) NOT NULL DEFAULT '0000',
  `skillName` varchar(45) NOT NULL DEFAULT 'NONE',
  `animForMe` varchar(10) NOT NULL DEFAULT '000000',
  `animForOpponentPVP` varchar(10) NOT NULL DEFAULT '000000',
  `animForOpponentPVE` varchar(10) NOT NULL DEFAULT '000000',
  `CAST_FX` varchar(10) NOT NULL DEFAULT 'FALSE',
  `FXForMe` varchar(10) NOT NULL DEFAULT '00000000',
  `FXForOpponentPVP` varchar(10) NOT NULL DEFAULT '00000000',
  `DAMAGE` int(11) NOT NULL DEFAULT '0',
  `DISTANCE` int(11) NOT NULL DEFAULT '0',
  `weaponForMe` varchar(5) NOT NULL DEFAULT '0000',
  PRIMARY KEY (`skillId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myteam` (
  `handle` varchar(45) NOT NULL,
  `IN_TEAM` int(11) NOT NULL DEFAULT '0',
  `COMMAND` varchar(45) NOT NULL DEFAULT 'NONE',
  `SLOTS` int(11) NOT NULL DEFAULT '50',
  `MEMBERS` varchar(15000) NOT NULL DEFAULT 'NONE',
  `CAPTAIN` varchar(45) NOT NULL DEFAULT 'NONE',
  `STATE` varchar(45) NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `mytotalnpcnumber` (
  `command` varchar(30) NOT NULL DEFAULT 'SPAWN',
  `npcNumber` int(11) NOT NULL COMMENT 'THIS-1',
  PRIMARY KEY (`command`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myuseitem` (
  `itemId` varchar(45) NOT NULL DEFAULT '00000000',
  `DO_THIS` varchar(1000) NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`itemId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `myvendorsandcollectors` (
  `npcId` varchar(45) NOT NULL DEFAULT '00000000',
  `npcName` varchar(45) NOT NULL DEFAULT 'UNKNOWN',
  `type` varchar(45) NOT NULL DEFAULT 'NONE' COMMENT 'RSI_PILLS, CLOTHES, WEAPONS, PROP, BOOKS, RSI_CAPTURES, RSI_DISGUISES, CONSUMABLES',
  `quantity` int(11) NOT NULL DEFAULT '0',
  `items` varchar(10000) NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`npcId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `pvpmode` (
  `handle` varchar(45) NOT NULL,
  `pvp` int(11) NOT NULL DEFAULT '0',
  `hp` int(11) NOT NULL DEFAULT '0',
  `is` int(11) NOT NULL DEFAULT '0',
  `lvl` int(11) NOT NULL DEFAULT '0',
  `animForMe` varchar(10) NOT NULL DEFAULT '000000',
  `animForOpponent` varchar(10) NOT NULL DEFAULT '000000',
  `damageForOpponent` int(11) NOT NULL DEFAULT '0',
  `HandleOpponent` varchar(45) NOT NULL DEFAULT '000000',
  `buffer` int(11) NOT NULL DEFAULT '0',
  `X_POS` int(11) NOT NULL DEFAULT '0',
  `Y_POS` int(11) NOT NULL DEFAULT '0',
  `Z_POS` int(11) NOT NULL DEFAULT '0',
  `STATE` varchar(45) NOT NULL DEFAULT 'NONE',
  `STATEB` varchar(45) NOT NULL DEFAULT 'NONE',
  `pve` int(11) NOT NULL DEFAULT '0',
  `IdOpponentPVE` varchar(5) NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`handle`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `rsivalues` (
  `charId` bigint(30) unsigned NOT NULL,
  `sex` smallint(6) NOT NULL,
  `body` smallint(6) NOT NULL,
  `hat` smallint(6) NOT NULL,
  `face` smallint(6) NOT NULL,
  `shirt` smallint(6) NOT NULL,
  `coat` smallint(6) NOT NULL,
  `pants` smallint(6) NOT NULL,
  `shoes` smallint(6) NOT NULL,
  `gloves` smallint(6) NOT NULL,
  `glasses` smallint(6) NOT NULL,
  `hair` smallint(6) NOT NULL,
  `facialdetail` smallint(6) NOT NULL,
  `shirtcolor` smallint(6) NOT NULL,
  `pantscolor` smallint(6) NOT NULL,
  `coatcolor` smallint(6) NOT NULL,
  `shoecolor` smallint(6) NOT NULL,
  `glassescolor` smallint(6) NOT NULL,
  `haircolor` smallint(6) NOT NULL,
  `skintone` smallint(6) NOT NULL,
  `tattoo` smallint(6) NOT NULL,
  `facialdetailcolor` smallint(6) NOT NULL,
  `leggings` smallint(6) NOT NULL,
  PRIMARY KEY (`charId`),
  UNIQUE KEY `charId` (`charId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `users` (
  `userId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `passwordSalt` varchar(32) NOT NULL,
  `passwordHash` varchar(40) NOT NULL,
  `publicExponent` smallint(11) unsigned NOT NULL DEFAULT '0',
  `publicModulus` tinyblob,
  `privateExponent` tinyblob,
  `timeCreated` int(10) unsigned NOT NULL,
  `account_status` int(11) NOT NULL DEFAULT '0' COMMENT 'if banned',
  `sessionid` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`userId`),
  UNIQUE KEY `id` (`userId`),
  UNIQUE KEY `username` (`username`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=389 ;

CREATE TABLE IF NOT EXISTS `worlds` (
  `worldId` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(20) NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT '1 for no pvp, 2 for pvp',
  `status` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT 'World Status (Down, Open etc.)',
  `load` tinyint(3) unsigned NOT NULL DEFAULT '49',
  `numPlayers` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`worldId`),
  UNIQUE KEY `worldId` (`worldId`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=5 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
