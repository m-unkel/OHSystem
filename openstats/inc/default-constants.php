<?php
/*********************************************
<!-- 
*   	DOTA OPENSTATS
*   
*	Developers: Ivan.
*	Contact: ivan.anta@gmail.com - Ivan
*
*	
*	Please see http://openstats.iz.rs
*	and post your webpage there, so I know who's using it.
*
*	Files downloaded from http://openstats.iz.rs
*
*	Copyright (C) 2010  Ivan
*
*
*	This file is part of DOTA OPENSTATS.
*
* 
*	 DOTA OPENSTATS is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    DOTA OPEN STATS is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with DOTA OPEN STATS.  If not, see <http://www.gnu.org/licenses/>
*
-->
**********************************************/
if (!isset($website) ) {header('HTTP/1.1 404 Not Found'); die; }

define('OS_VERSION', '4.0.0.1');
define('OS_HOME',      $website); 
define('OS_HOME_TITLE',    $HomeTitle);
define('OS_TIMEZONE',      $TimeZone);
define('OS_BOTNAME',       $BotName);

define('OSDB_ADMINS',   'oh_admins'); //do not change
define('OSDB_APPEALS',  'oh_ban_appeals');
define('OSDB_REPORTS',  'oh_ban_reports');
define('OSDB_BANS',     'oh_bans'); //do not change
define('OSDB_COMMANDS_QUEUE', 'oh_commands_queue'); //do not change
define('OSDB_COMMENTS', 'oh_comments');
define('OSDB_CUSTOM_FIELDS',  'oh_custom_fields');
define('OSDB_DG',       'oh_dotagames'); //do not change
define('OSDB_DP',       'oh_dotaplayers'); //do not change
define('OSDB_DL',       'oh_downloads'); //do not change

define('OSDB_ADMIN_LOG', 'oh_adminlog'); //do not change
define('OSDB_GAME_INFO', 'oh_game_info'); //do not change
define('OSDB_GAMELOG',   'oh_game_log'); //do not change
define('OSDB_GO',        'oh_game_offenses'); //do not change
define('OSDB_GAMESTATUS','oh_game_status'); //do not change
define('OSDB_GAMELIST',  'oh_gamelist'); //table for gamelist patch
define('OSDB_GP',        'oh_gameplayers'); //do not change
define('OSDB_GPR',       'oh_gameplayers_rating');
define('OSDB_GAMES',     'oh_games'); //do not change

define('OSDB_GUIDES',   'oh_hero_guides');
define('OSDB_HEROES',   'oh_heroes');
define('OSDB_ITEMS',    'oh_items');

define('OSDB_NEWS',        'oh_news');
define('OSDB_PLAYER_INFO', 'oh_playerinfo');

define('OSDB_BNET_PM',     'oh_pm'); //do not change
define('OSDB_PM',     'oh_pm'); //do not change

define('OSDB_STATS',         'oh_stats');
define('OSDB_STATS_M',       'oh_stats_monthly');
define('OSDB_STATS_P',       'oh_stats_players');
define('OSDB_USERS',         'oh_users');



define('OSDB_NOTES',       'oh_notes');
define('OSDB_PERMISSIONS', 'oh_permissions');
//define('OSDB_SAFELIST',    'oh_safelist'); //do not change
define('OSDB_SCORES',      'oh_scores');


//define('OSDB_VISITORS',    'visitors');
define('OSDB_W3PL',        'oh_w3mmdplayers'); //do not change
define('OSDB_W3VARS',      'oh_w3mmdvars'); //do not change

//new
define('OSDB_ALIASES',    'oh_aliases');
define('OSDB_GOALS',      'oh_goals');
define('OSDB_GOALS_LOG',  'oh_goals_log');
define('OSDB_LG_LOGS',    'oh_lobby_game_logs');
define('OSDB_GPROXY',     'oh_gproxy'); //do not change


//DATABASE
define('OSDB_SERVER',      $server); 
define('OSDB_USERNAME',    $username); 
define('OSDB_PASSWORD',    $password); 
define('OSDB_DATABASE',    $database); 

define('OS_THEMES_DIR',              $DefaultStyle);
define('OS_THEME_PATH',              OS_HOME."themes/".OS_THEMES_DIR."/");
define('OS_CURRENT_THEME_PATH',     "themes/".OS_THEMES_DIR."/"); 
define('OS_PLUGINS_DIR',            'plugins/');
define('OS_PAGE_PATH',               "inc/pages/"); 
define('OS_LANGUAGE',                $default_language);
define('OS_DATE_FORMAT',             $DateFormat); 
define('OS_MIN_GAME_DURATION',       $MinDuration);
define('OS_DEFAULT_MAP',             $DefaultMap);

//
define('OS_TOP_ENABLED',       $TopPage);
define('OS_HEROES_ENABLED',    $HeroesPage);
define('OS_TOP_ONLINE',        $OnlineOfflineOnTopPage);
//
?>
