SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

DROP TABLE IF EXISTS `cron_logs`;
CREATE TABLE IF NOT EXISTS `cron_logs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `cron_data` text NOT NULL,
  `cron_date` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 collate utf8_general_ci;

DROP TABLE IF EXISTS `oh_adminlog`;
CREATE TABLE IF NOT EXISTS `oh_adminlog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `botid` int(11) NOT NULL,
  `gameid` int(11) NOT NULL,
  `log_time` datetime NOT NULL,
  `log_admin` varchar(30) NOT NULL,
  `log_data` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `time` (`log_time`),
  KEY `gameid` (`gameid`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_aliases`;
CREATE TABLE IF NOT EXISTS `oh_aliases` (
  `alias_id` int(11) NOT NULL,
  `alias_name` varchar(255) NOT NULL,
  `default_alias` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`alias_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

INSERT INTO `oh_aliases` (`alias_id`, `alias_name`, `default_alias`) VALUES (1, 'DotA', 1);

DROP TABLE IF EXISTS `oh_bans`;
CREATE TABLE IF NOT EXISTS `oh_bans` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `botid` int(11) unsigned NOT NULL,
  `alias_id` smallint(6) unsigned NOT NULL,
  `server` varchar(100) NOT NULL,
  `name` varchar(15) NOT NULL,
  `ip` varchar(16) NOT NULL,
  `ip_part` varchar(10) NOT NULL,
  `country` varchar(4) NOT NULL,
  `date` datetime NOT NULL,
  `gamename` varchar(31) NOT NULL,
  `admin` varchar(15) NOT NULL,
  `reason` varchar(255) NOT NULL,
  `gamecount` int(11) unsigned NOT NULL,
  `expiredate` datetime NOT NULL,
  `warn` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`),
  KEY `date` (`date`),
  KEY `expire` (`expiredate`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_ban_appeals`;
CREATE TABLE IF NOT EXISTS `oh_ban_appeals` (
  `player_id` int(11) NOT NULL,
  `player_name` varchar(25) NOT NULL,
  `user_id` int(11) NOT NULL,
  `user_ip` varchar(20) NOT NULL,
  `reason` varchar(255) NOT NULL,
  `game_url` varchar(200) NOT NULL,
  `replay_url` varchar(255) NOT NULL,
  `added` int(11) NOT NULL,
  `status` tinyint(4) NOT NULL,
  `resolved` varchar(30) NOT NULL,
  `resolved_text` varchar(255) NOT NULL,
  KEY `player_id` (`player_id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_ban_reports`;
CREATE TABLE IF NOT EXISTS `oh_ban_reports` (
  `player_id` int(11) NOT NULL,
  `player_name` varchar(25) NOT NULL,
  `user_id` int(11) NOT NULL,
  `user_ip` varchar(20) NOT NULL,
  `reason` varchar(255) NOT NULL,
  `game_url` varchar(200) NOT NULL,
  `replay_url` varchar(255) NOT NULL,
  `added` int(11) NOT NULL,
  `status` tinyint(4) NOT NULL,
  KEY `player_id` (`player_id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `oh_bot_status` (
  `botid` tinyint(4) NOT NULL,
  `name` varchar(15) NOT NULL,
  `gamename` varchar(31) NOT NULL,
  `ip` varchar(21) NOT NULL,
  `hostport` smallint(6) unsigned NOT NULL,
  `roc` varchar(50) NOT NULL,
  `tft` varchar(50) NOT NULL,
  `last_update` datetime NOT NULL,
  `Europe` tinyint(1) unsigned NOT NULL,
  `USEast` tinyint(1) unsigned NOT NULL,
  `USWest` tinyint(1) unsigned NOT NULL,
  `Asia` tinyint(1) unsigned NOT NULL,
  `XAMP` tinyint(1) unsigned NOT NULL,
  `Garena` tinyint(4) NOT NULL,
  `WC3Connect` tinyint(4) NOT NULL,
  PRIMARY KEY (`botid`),
  KEY `hostport` (`hostport`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_bot_configuration`;
CREATE TABLE IF NOT EXISTS `oh_bot_configuration` (
  `cfg_botid` tinyint(4) NOT NULL,
  `cfg_name` varchar(150) NOT NULL,
  `cfg_description` varchar(255) NOT NULL,
  `cfg_value` varchar(100) NOT NULL,
  KEY `cfg_name` (`cfg_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_commands_log`;
CREATE TABLE IF NOT EXISTS `oh_commands_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `botid` tinyint(2) unsigned DEFAULT NULL,
  `userid` int(10) UNSIGNED DEFAULT NULL,
  `command` varchar(1024) DEFAULT NULL,
  `executed` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_commands_queue`;
CREATE TABLE IF NOT EXISTS `oh_commands_queue` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `botid` tinyint(2) unsigned DEFAULT NULL,
  `userid` int(10) UNSIGNED DEFAULT NULL,
  `command` varchar(1024) DEFAULT NULL,
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_comments`;
CREATE TABLE IF NOT EXISTS `oh_comments` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(11) unsigned NOT NULL,
  `page` varchar(16) NOT NULL,
  `post_id` int(11) unsigned NOT NULL,
  `text` text NOT NULL,
  `date` int(11) NOT NULL,
  `user_ip` varchar(16) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_custom_fields`;
CREATE TABLE IF NOT EXISTS `oh_custom_fields` (
  `field_id` int(11) NOT NULL,
  `field_name` varchar(64) NOT NULL,
  `field_value` longtext NOT NULL,
  KEY `field_id` (`field_id`,`field_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_dotagames`;
CREATE TABLE IF NOT EXISTS `oh_dotagames` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `botid` smallint(4) unsigned NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `winner` tinyint(1) unsigned NOT NULL,
  `min` smallint(4) unsigned NOT NULL,
  `sec` tinyint(2) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `gameid` (`gameid`),
  KEY `winner` (`winner`),
  KEY `min` (`min`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_dotaplayers`;
CREATE TABLE IF NOT EXISTS `oh_dotaplayers` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `botid` smallint(4) unsigned NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `colour` tinyint(1) unsigned NOT NULL,
  `kills` smallint(4) NOT NULL,
  `deaths` smallint(4) unsigned NOT NULL,
  `creepkills` smallint(4) unsigned NOT NULL,
  `creepdenies` smallint(4) unsigned NOT NULL,
  `assists` smallint(4) unsigned NOT NULL,
  `gold` int(11) unsigned NOT NULL,
  `neutralkills` smallint(4) unsigned NOT NULL,
  `item1` char(4) NOT NULL,
  `item2` char(4) NOT NULL,
  `item3` char(4) NOT NULL,
  `item4` char(4) NOT NULL,
  `item5` char(4) NOT NULL,
  `item6` char(4) NOT NULL,
  `spell1` char(4) NOT NULL,
  `spell2` char(4) NOT NULL,
  `spell3` char(4) NOT NULL,
  `spell4` char(4) NOT NULL,
  `spell5` char(4) NOT NULL,
  `spell6` char(4) NOT NULL,
  `hero` char(4) NOT NULL,
  `newcolour` tinyint(1) unsigned NOT NULL,
  `towerkills` tinyint(2) unsigned NOT NULL,
  `raxkills` tinyint(1) unsigned NOT NULL,
  `courierkills` tinyint(1) unsigned NOT NULL,
  `level` tinyint(2) unsigned NOT NULL,
  `apm` tinyint(2) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `gameid` (`gameid`,`colour`),
  KEY `colour` (`colour`),
  KEY `newcolour` (`newcolour`),
  KEY `hero` (`hero`),
  KEY `item1` (`item1`),
  KEY `item2` (`item2`),
  KEY `item3` (`item3`),
  KEY `item4` (`item4`),
  KEY `item5` (`item5`),
  KEY `item6` (`item6`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_downloads`;
CREATE TABLE IF NOT EXISTS `oh_downloads` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `botid` int(11) NOT NULL,
  `map` varchar(100) NOT NULL,
  `mapsize` int(11) NOT NULL,
  `datetime` datetime NOT NULL,
  `name` varchar(15) NOT NULL,
  `ip` varchar(15) NOT NULL,
  `spoofed` int(11) NOT NULL,
  `spoofedrealm` varchar(100) NOT NULL,
  `downloadtime` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `oh_goals` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `goal` tinyint(4) NOT NULL,
  `min_games` smallint(6) NOT NULL,
  `start_date` datetime NOT NULL,
  `end_date` datetime NOT NULL,
  `reward` smallint(6) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `start` (`start_date`),
  KEY `end` (`end_date`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_gamelist`;

CREATE TABLE IF NOT EXISTS `oh_gamelist` (
  `botid` smallint(4) unsigned NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `lobby` tinyint(1) NOT NULL,
  `duration` int(11) NOT NULL,
  `map_type` varchar(50) NOT NULL,
  `gamename` varchar(30) NOT NULL,
  `ownername` varchar(15) NOT NULL,
  `creatorname` varchar(15) NOT NULL,
  `map` varchar(100) NOT NULL,
  `users` varchar(1000) NOT NULL,
  `players` smallint(6) NOT NULL,
  `total` smallint(6) NOT NULL,
  PRIMARY KEY (`botid`),
  KEY `gameid` (`gameid`),
  KEY `lobby` (`lobby`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

DROP TABLE IF EXISTS `oh_gameplayers`;
CREATE TABLE IF NOT EXISTS `oh_gameplayers` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `player_id` int(11) unsigned NOT NULL,
  `botid` smallint(4) unsigned NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `name` varchar(15) NOT NULL,
  `ip` varchar(15) NOT NULL,
  `spoofed` tinyint(1) unsigned NOT NULL,
  `reserved` tinyint(1) unsigned NOT NULL,
  `loadingtime` int(11) unsigned NOT NULL,
  `left` int(11) NOT NULL,
  `leftreason` varchar(100) NOT NULL,
  `team` tinyint(1) unsigned NOT NULL,
  `colour` tinyint(1) unsigned NOT NULL,
  `spoofedrealm` varchar(100) NOT NULL,
  `score_before` int(10) NOT NULL,
  `score_after` int(10) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `gameid` (`gameid`),
  KEY `colour` (`colour`),
  KEY `name` (`name`),
  KEY `playerID` (`player_id`),
  KEY `team` (`team`),
  KEY `ip` (`ip`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_gameplayers_rating`;
CREATE TABLE IF NOT EXISTS `oh_gameplayers_rating` (
  `player` varchar(30) NOT NULL,
  `voter` varchar(30) NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `time` int(11) unsigned NOT NULL,
  `rate` tinyint(2) NOT NULL,
  `comment` varchar(255) NOT NULL,
  KEY `pid` (`player`),
  KEY `rate` (`rate`),
  KEY `gameid` (`gameid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_games`;
CREATE TABLE IF NOT EXISTS `oh_games` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `botid` smallint(4) unsigned NOT NULL,
  `alias_id` smallint(4) unsigned NOT NULL,
  `server` varchar(100) NOT NULL,
  `map` varchar(100) NOT NULL,
  `datetime` datetime NOT NULL,
  `gamename` varchar(31) NOT NULL,
  `ownername` varchar(15) NOT NULL,
  `duration` int(11) unsigned NOT NULL,
  `gamestate` tinyint(1) unsigned NOT NULL,
  `creatorname` varchar(15) NOT NULL,
  `creatorserver` varchar(20) NOT NULL,
  `gametype` tinyint(2) unsigned NOT NULL,
  `stats` tinyint(1) unsigned NOT NULL,
  `views` int(11) unsigned NOT NULL,
  `gamestatus` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `datetime` (`datetime`),
  KEY `map` (`map`),
  KEY `duration` (`duration`),
  KEY `gamestate` (`gamestate`),
  KEY `alias_id` (`alias_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_game_info`;
CREATE TABLE IF NOT EXISTS `oh_game_info` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `botid` smallint(4) unsigned NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `server` varchar(100) NOT NULL,
  `map` varchar(100) NOT NULL,
  `datetime` datetime NOT NULL,
  `gamename` varchar(31) NOT NULL,
  `ownername` varchar(30) NOT NULL,
  `duration` int(11) unsigned NOT NULL,
  `lobbytime` int(11) unsigned NOT NULL,
  `gamestate` int(11) unsigned NOT NULL,
  `creatorname` varchar(30) NOT NULL,
  `creatorserver` varchar(100) NOT NULL,
  `gametype` tinyint(1) unsigned NOT NULL,
  `winner` tinyint(1) unsigned NOT NULL,
  `min` tinyint(1) unsigned NOT NULL,
  `sec` tinyint(1) unsigned NOT NULL,
  `stats` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_game_log`;
CREATE TABLE IF NOT EXISTS `oh_game_log` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `botid` smallint(4) unsigned NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `log_time` datetime NOT NULL,
  `log_data` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `time` (`log_time`),
  KEY `gameid` (`gameid`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_game_offenses`;
CREATE TABLE IF NOT EXISTS `oh_game_offenses` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `player_id` int(11) unsigned NOT NULL,
  `player_name` varchar(30) NOT NULL,
  `reason` varchar(255) NOT NULL,
  `offence_time` datetime NOT NULL,
  `offence_expire` datetime NOT NULL,
  `pp` tinyint(2) NOT NULL,
  `admin` varchar(30) NOT NULL,
  `banned` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `pid` (`player_id`),
  KEY `time` (`offence_time`),
  KEY `expires` (`offence_expire`),
  KEY `pp` (`pp`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_game_status`;
CREATE TABLE IF NOT EXISTS `oh_game_status` (
  `botid` int(11) unsigned NOT NULL,
  `gameid` int(11) unsigned NOT NULL,
  `alias_id` smallint(4) unsigned NOT NULL,
  `gametime` datetime NOT NULL,
  `gamename` varchar(30) NOT NULL,
  `gamestatus` tinyint(2) unsigned NOT NULL,
  `gametype` tinyint(1) unsigned NOT NULL DEFAULT '0',
  KEY `botid` (`botid`),
  KEY `gameid` (`gameid`),
  KEY `status` (`gamestatus`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_geoip`;
CREATE TABLE IF NOT EXISTS `oh_geoip` (
  `ip_start` varchar(16) NOT NULL,
  `ip_end` varchar(16) NOT NULL,
  `ip_start_int` int(11) NOT NULL,
  `ip_end_int` int(11) NOT NULL,
  `code` varchar(4) NOT NULL,
  `country` varchar(30) NOT NULL,
  KEY `ip_start` (`ip_start`,`ip_end`),
  KEY `code` (`code`),
  KEY `ip_integer` (`ip_start_int`,`ip_end_int`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_gproxy`;
CREATE TABLE IF NOT EXISTS `oh_gproxy` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `player` varchar(30) NOT NULL,
  `ip` varchar(20) NOT NULL,
  `added` datetime NOT NULL,
  `added_by` varchar(20) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `player` (`player`),
  KEY `ip` (`ip`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_heroes`;
CREATE TABLE IF NOT EXISTS `oh_heroes` (
  `heroid` varchar(4) NOT NULL,
  `original` varchar(4) NOT NULL,
  `description` varchar(32) NOT NULL,
  `playcount` int(11) NOT NULL,
  `wins` int(11) NOT NULL,
  `summary` varchar(900) NOT NULL,
  `stats` varchar(300) NOT NULL,
  `skills` varchar(300) NOT NULL,
  `type` tinyint(4) NOT NULL,
  `voteup` int(11) NOT NULL DEFAULT '0',
  `votedown` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`heroid`),
  KEY `description` (`description`),
  KEY `original` (`original`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_hero_guides`;
CREATE TABLE IF NOT EXISTS `oh_hero_guides` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `hid` varchar(6) NOT NULL,
  `title` varchar(255) NOT NULL,
  `link` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `hid` (`hid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_items`;
CREATE TABLE IF NOT EXISTS `oh_items` (
  `itemid` varchar(4) NOT NULL,
  `playcount` int(11) NOT NULL,
  `code` smallint(10) NOT NULL,
  `name` varchar(50) NOT NULL,
  `shortname` varchar(50) NOT NULL,
  `item_info` mediumtext NOT NULL,
  `price` smallint(6) NOT NULL,
  `type` varchar(10) NOT NULL,
  `icon` varchar(50) NOT NULL,
  PRIMARY KEY (`itemid`),
  KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_lobby_game_logs`;
CREATE TABLE IF NOT EXISTS `oh_lobby_game_logs` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `gameid` int(11) unsigned NOT NULL,
  `botid` smallint(4) unsigned NOT NULL,
  `gametype` tinyint(1) unsigned NOT NULL,
  `lobbylog` longtext NOT NULL,
  `gamelog` longtext NOT NULL,
  PRIMARY KEY (`id`),
  KEY `gameid` (`gameid`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_news`;
CREATE TABLE IF NOT EXISTS `oh_news` (
  `news_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `news_title` varchar(255) NOT NULL,
  `news_content` mediumtext NOT NULL,
  `author` int(11) unsigned NOT NULL,
  `comments` int(11) unsigned NOT NULL,
  `news_date` int(11) unsigned NOT NULL,
  `news_updated` int(11) unsigned NOT NULL,
  `views` int(11) unsigned NOT NULL,
  `status` tinyint(1) unsigned NOT NULL,
  `allow_comments` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`news_id`),
  KEY `status` (`status`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_pm`;
CREATE TABLE IF NOT EXISTS `oh_pm` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `m_from` varchar(30) NOT NULL,
  `m_to` varchar(30) NOT NULL,
  `m_time` datetime NOT NULL,
  `m_read` tinyint(1) unsigned NOT NULL,
  `m_message` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `from` (`m_from`),
  KEY `to` (`m_to`),
  KEY `status` (`m_read`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_stats`;
CREATE TABLE IF NOT EXISTS `oh_stats` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `pid` int(11) unsigned NOT NULL,
  `alias_id` smallint(11) unsigned NOT NULL,
  `player` varchar(30) NOT NULL,
  `player_lower` varchar(30) NOT NULL,
  `last_seen` datetime NOT NULL,
  `country_code` varchar(4) NOT NULL,
  `country` varchar(40) NOT NULL,
  `month` varchar(2) NOT NULL DEFAULT '1',
  `year` year(4) NOT NULL,
  `user_level` tinyint(2) unsigned NOT NULL,
  `forced_gproxy` tinyint(1) unsigned NOT NULL,
  `score` int(11) NOT NULL,
  `games` int(11) NOT NULL,
  `wins` int(11) NOT NULL,
  `losses` int(11) NOT NULL,
  `draw` int(11) NOT NULL,
  `kills` int(11) NOT NULL,
  `deaths` int(11) NOT NULL,
  `assists` int(11) NOT NULL,
  `creeps` int(11) NOT NULL,
  `denies` int(11) NOT NULL,
  `neutrals` int(11) NOT NULL,
  `towers` int(11) NOT NULL,
  `rax` int(11) NOT NULL,
  `banned` tinyint(1) unsigned NOT NULL,
  `realm` varchar(100) NOT NULL,
  `reserved` int(11) unsigned NOT NULL,
  `leaver` int(11) unsigned NOT NULL,
  `ip` varchar(16) NOT NULL,
  `streak` int(11) unsigned NOT NULL,
  `maxstreak` int(11) unsigned NOT NULL,
  `losingstreak` int(11) unsigned NOT NULL,
  `maxlosingstreak` int(11) unsigned NOT NULL,
  `zerodeaths` int(11) unsigned NOT NULL,
  `points` int(11) unsigned NOT NULL,
  `points_bet` tinyint(11) unsigned NOT NULL,
  `hide` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `updated` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `games` (`games`),
  KEY `wins` (`wins`),
  KEY `losses` (`losses`),
  KEY `draw` (`draw`),
  KEY `kills` (`kills`),
  KEY `deaths` (`deaths`),
  KEY `assists` (`assists`),
  KEY `ck` (`creeps`),
  KEY `cd` (`denies`),
  KEY `player` (`player`),
  KEY `player_lower` (`player_lower`),
  KEY `country_code` (`country_code`),
  KEY `month` (`month`),
  KEY `year` (`year`),
  KEY `hide` (`hide`),
  KEY `pid` (`pid`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_stats_players`;
CREATE TABLE IF NOT EXISTS `oh_stats_players` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `player` varchar(50) NOT NULL,
  `player_lower` varchar(50) NOT NULL,
  `user_level` tinyint(1) unsigned NOT NULL,
  `last_seen` datetime NOT NULL,
  `banned` tinyint(1) unsigned NOT NULL,
  `hide` tinyint(1) unsigned NOT NULL,
  `realm` varchar(20) NOT NULL,
  `ip` varchar(15) NOT NULL,
  `leaver_level` tinyint(4) NOT NULL DEFAULT '0',
  `last_leaver_time` datetime NOT NULL,
  `country` varchar(50) NOT NULL,
  `country_code` varchar(2) NOT NULL,
  `points` int(11) unsigned NOT NULL,
  `points_bet` tinyint(6) unsigned NOT NULL,
  `blacklisted` tinyint(1) unsigned NOT NULL,
  `exp` int(10) unsigned NOT NULL,
  `player_language` varchar(2) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `player` (`player`),
  KEY `country_code` (`country_code`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_users`;
CREATE TABLE IF NOT EXISTS `oh_users` (
  `user_id` mediumint(8) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(60) NOT NULL,
  `alias_id` smallint(6) unsigned NOT NULL,
  `bnet_username` varchar(30) NOT NULL,
  `user_bnet` tinyint(1) unsigned NOT NULL,
  `blacklisted` tinyint(1) unsigned NOT NULL,
  `user_password` varchar(100) NOT NULL,
  `password_hash` varchar(65) NOT NULL,
  `user_ppwd` varchar(20) NOT NULL,
  `user_email` varchar(60) NOT NULL,
  `user_color` varchar(6) NOT NULL DEFAULT 'FFFFFF',
  `user_joined` int(11) unsigned NOT NULL DEFAULT '0',
  `user_level` tinyint(1) unsigned NOT NULL,
  `user_custom_permission` varchar(10) NOT NULL DEFAULT '0000000000',
  `user_level_expire` datetime NOT NULL,
  `admin_realm` varchar(64) NOT NULL,
  `expire_date` datetime NOT NULL,
  `user_last_login` int(11) unsigned NOT NULL DEFAULT '0',
  `user_points_time` int(11) unsigned NOT NULL,
  `user_ip` varchar(40) NOT NULL,
  `user_avatar` varchar(255) NOT NULL,
  `user_location` varchar(100) NOT NULL,
  `user_realm` varchar(50) NOT NULL,
  `user_website` varchar(255) NOT NULL,
  `user_gender` tinyint(1) unsigned NOT NULL,
  `user_lang` varchar(30) NOT NULL,
  `user_clan` varchar(30) NOT NULL,
  `user_fbid` varchar(30) NOT NULL,
  `phpbb_id` int(11) unsigned NOT NULL,
  `smf_id` int(11) unsigned NOT NULL,
  `can_comment` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `code` varchar(15) NOT NULL,
  `confirm` varchar(65) NOT NULL,
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `user_name` (`user_name`),
  KEY `last_login` (`user_last_login`),
  KEY `joined` (`user_joined`),
  KEY `confirm` (`confirm`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_w3mmdplayers`;
CREATE TABLE IF NOT EXISTS `oh_w3mmdplayers` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `botid` int(11) NOT NULL,
  `category` varchar(25) NOT NULL,
  `gameid` int(11) NOT NULL,
  `pid` int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  `flag` varchar(32) NOT NULL,
  `leaver` int(11) NOT NULL,
  `practicing` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `gameid` (`gameid`),
  KEY `pid` (`pid`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `oh_w3mmdvars`;
CREATE TABLE IF NOT EXISTS `oh_w3mmdvars` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `botid` int(11) NOT NULL,
  `gameid` int(11) NOT NULL,
  `pid` int(11) NOT NULL,
  `varname` varchar(25) NOT NULL,
  `value_int` int(11) DEFAULT NULL,
  `value_real` double DEFAULT NULL,
  `value_string` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `gameid` (`gameid`),
  KEY `pid` (`pid`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `wc3connect`;
CREATE TABLE IF NOT EXISTS `wc3connect` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(256) DEFAULT NULL,
  `user_id` int(11) NOT NULL,
  `user_ip` varchar(20) DEFAULT NULL,
  `sessionkey` mediumtext,
  `time` datetime DEFAULT NULL,
  `proxy` varchar(128) DEFAULT NULL,
  `startalert` int(11) DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `wc3connect_list`;
CREATE TABLE IF NOT EXISTS `wc3connect_list` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `ipport` varchar(64) DEFAULT NULL,
  `data` varchar(1024) DEFAULT NULL,
  `time` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;
