<?php
  //if (strstr($_SERVER['REQUEST_URI'], basename(__FILE__) ) ) {header('HTTP/1.1 404 Not Found'); die; }
  include("config.php");

  //$TotalLogs = 20000; //total log entries to keep in database. We will add this variable to config.

  if ( isset($_SESSION["die"]) ) die;
 
  
  require_once('inc/common.php');
  
  if ( !strstr( $_SERVER['HTTP_REFERER'], OS_HOME ) ) { header('HTTP/1.1 404 Not Found'); die; }
  
  require_once('lang/'.OS_LANGUAGE.'.php');

  if ( isset($_SESSION["restricted"]) AND $_SESSION["restricted"]+10>time() ) { ?>
  <img src="<?=OS_HOME?>img/warn.png" alt="" class="imgvalign" /> <?=$lang["gl_no_accesss"]?>
  <?php  
  //$_SESSION["die"] = true;
  die;
  }
  
  require_once('inc/class.db.PDO.php'); 
  require_once('inc/db_connect.php');
  require_once('inc/live_games_functions.php');
  
   foreach($_POST as $key => $value) {
    $_POST[$key] = FilterData($value);
   }

   //GET GAME LISTS
   if (isset($_POST["refresh"]) AND $_POST["refresh"] == "gamelist" ) {

    if ($LiveGamesLimit>=1) $LIMIT = "LIMIT $LiveGamesLimit";
	
	if (isset($_POST["gameid"]) AND $_POST["gameid"]>=1 ) {
	 $selectedGameID = (int)$_POST["gameid"];
	} else $selectedGameID = "";
	
	$sql = " ";
	//if (os_is_logged() AND $_SESSION["level"]>=9 ) $sql = " ";
	//else $sql = " AND botid = 1";
	
	if ( isset($_POST["alias_id"]) AND is_numeric($_POST["alias_id"]))
	$sql.= " AND alias_id = '".(int)$_POST["alias_id"]."' OR alias_id>=200";
	
    $sth = $db->prepare( "SELECT * FROM ".OSDB_GAMESTATUS." 
	WHERE gamestatus<=2 
	AND gametime>=NOW()-INTERVAL 2 hour
	$sql
	GROUP BY gameid
    ORDER BY gamestatus ASC, gametime DESC 
	$LIMIT" );
    $result = $sth->execute();
    $IDS = array();
    $c = 0;
     while ($row = $sth->fetch(PDO::FETCH_ASSOC)) {
	$botID = $row["botid"];
	$chatID = $row["gameid"];
	$GameStatus = $row["gamestatus"];
	$GameTime = $row["gametime"];
	$GameName = os_strip_quotes($row["gamename"]);
      if ($chatID>=1 ) { 
	  $IDS[$c]["chatid"] = $chatID;
	  if ($GameStatus <= 1 ) { $pr = "";$gamebutton = " LobbyButton"; }
	  if ($GameStatus == 2 ) { $pr = "G"; $gamebutton = " GameButton"; }
	  $IDS[$c]["chatid"] = $chatID; 
	  $IDS[$c]["status"] = $pr; 
	  $IDS[$c]["button"] = $gamebutton;
	  $IDS[$c]["gn"] = $GameName;
	  $IDS[$c]["botid"] = $botID; 
	  $c++; 
	  }
    }
?>
<span id="gamerefresher" class="h32"><img src="<?=OS_HOME?>/img/blank.gif" width="16" height="16" class="imgvalign" /></span>
<?php	
  foreach($IDS as $cID) {
  if ( $cID["chatid"] == $selectedGameID) $dis = 'style="color:#E5B16D !important; font-weight:bold;"'; else $dis = '';
  ?>
<input <?=$dis?> id="b<?=$cID["chatid"]?>" type="button" class="menuButtons<?=$cID["button"]?> btn btn-xs btn-primary" onclick="clearLiveGamesTimer('<?=$cID["botid"]?>', '<?=$cID["chatid"]?>', '<?=$cID["gn"]?>');" value="<?=$cID["status"]?>#<?=$cID["gn"]?>" />
  <?php
  }

   } 
   
   //GET GAME DATA
   else if ( isset($_POST["botID"]) AND isset($_POST["lastID"]) AND isset($_POST["chatID"]) ) {
   
    $botID = (int) $_POST["botID"];
	$lastID =(int) $_POST["lastID"];
	$chatID =(int) $_POST["chatID"]; //GameID
	$click  =(int) $_POST["cl"];
	$UserIP = strip_tags($_SERVER["REMOTE_ADDR"]);
	//Check userIP (if user already in game). If so, don't show him live games.
	$sth = $db->prepare("SELECT COUNT(*) FROM ".OSDB_GP." 
	WHERE ip = '".$UserIP."' AND gameid = '".$chatID."' ");
	$result = $sth->execute();
	$r = $sth->fetch(PDO::FETCH_NUM);
	$numrows = $r[0];
	
	if ( $numrows>=1  ) {
	?><div><?=$lang["live_games_disable"]?></div><?php
	die();
	}

	//PURGE LOGS
	if ( $lastID>=$LiveGamesTotalLogs) 	{
	
	$sth = $db->prepare("SELECT COUNT(*) FROM ".OSDB_GAMELOG." LIMIT 1");
	$result = $sth->execute();
	$r = $sth->fetch(PDO::FETCH_NUM);
	$numrows = $r[0];
	
	//Somebody trying to "hack"?
	if ( $lastID+2>$numrows ) {
	die;
	//$_SESSION["restricted"] = time();
	}

	//Check total logs
	if ( $numrows >= $LiveGamesTotalLogs ) {
	
	$sth = $db->prepare("TRUNCATE TABLE ".OSDB_GAMELOG." ");
	$result = $sth->execute();
	}
	
	die;
	}
	
	$GetTime = date("Y-m-d H:i:s", time()-5 );
	
	if ( $lastID <=0 ) $sql = "SELECT * FROM `".OSDB_GAMELOG."` 
	WHERE botid=$botID AND id>$lastID AND gameid = '".$chatID."'
	ORDER BY id DESC LIMIT 1"; 
	else 
	$sql = "SELECT * FROM `".OSDB_GAMELOG."` 
	WHERE botid=$botID 
	AND id>$lastID 
	AND gameid = $chatID 
	ORDER BY id ASC, log_time ASC 
	LIMIT 1"; //get next log

	
	$sth = $db->prepare($sql);
	$result = $sth->execute();
	
	if ( $lastID>=$LiveGamesTotalLogs ) $PurgeLogs = 1;
	
	if ( $sth->rowCount()<=0 AND !isset($PurgeLogs) ) die;
	
	//Create Playerlist first time
	if (isset($PlayerListDataArray) ) unset($PlayerListDataArray);
	if ( $click == 1) {
	
	$sth2 = $db->prepare( "SELECT * FROM `".OSDB_GAMELOG."` 
	WHERE gameid = $chatID 
	AND log_data LIKE ('%	pl	%')
	ORDER BY id DESC, log_time DESC 
	LIMIT 1");
    $result = $sth2->execute();
	
	$row2 = $sth2->fetch(PDO::FETCH_ASSOC);
	$PlayerListDataArray = array_reverse ( explode("\n", $row2["log_data"]) );

	}

	while ($row = $sth->fetch(PDO::FETCH_ASSOC)) {
	$log_time = date( "d.m.Y, H:i:s", strtotime($row["log_time"]) );
	//$log_type = $row["log_type"];
	$log_data = $row["log_data"];
	$ID = $row["id"];
	
	//Reverse array to sort new logs on top
	$DataArray = array_reverse ( explode("\n", $log_data) );
	if (!isset($PlayerListDataArray) ) { $PlayerListDataArray = $DataArray; }
	echo OS_ParseGameLog( $log_data );
	} 
	?>|split|<?=($ID)?>|split|<?=OS_GetPlayerList( $PlayerListDataArray, $chatID, $lastID, $click )?><?php
   } 
   else if ( isset($_POST["winchance"]) AND is_numeric($_POST["winchance"]) ) {
   
   //WINCHANCE 
   
   	$gameid = (int) $_POST["winchance"];
	
	if ( file_exists("inc/geoip/geoip.inc") ) {
	include("inc/geoip/geoip.inc");
	$GeoIPDatabase = geoip_open("inc/geoip/GeoIP.dat", GEOIP_STANDARD);
	$GeoIP = 1;
	}
		
	$sth = $db->prepare( "SELECT * FROM `".OSDB_GAMELOG."` 
	WHERE id>1
	AND gameid = $gameid 
	AND log_data LIKE ('%	pl	%')
	ORDER BY id DESC, log_time DESC 
	LIMIT 1");
    $result = $sth->execute();
	
	$row = $sth->fetch(PDO::FETCH_ASSOC);
       
	 $log_time = date( "d.m.Y, H:i:s", strtotime($row["log_time"]) );
	 $log_data = $row["log_data"];
	 
	 $AllData = explode("\n", $log_data);
	 $DataArray = array_reverse ( $AllData );
	 //$PlayersInGame = array();
	 foreach( $DataArray as $Data) {
	   
	   $Log = explode("	", $Data);
	   if (isset($Log[0]) AND $Log[0]!= '-' ) $info  = $Log[0]; else $info = "";
       if (isset($Log[1]) AND $Log[1]!= '-' ) $event = $Log[1]; else $event = "";
	   if ( $event == "pl") {
	    $Logs = explode("	", $Data);
	   }
	 
	 }
	 
	 if ( !empty($Logs) ) {
	    $PlayerList = array();
	    for ($i = 1; $i <= 10; $i++) {
		if (isset($Logs[$i+1]) AND $Logs[$i+1]!= '-' ) $PlayerList[$i] = $Logs[$i+1]; else $PlayerList[$i] = "";
		}
	 }
	 
	 $ListPlayersData = array();
	 $c=1;
	 $Calculate = 0;
	 if (!empty($PlayerList) ) foreach ( $PlayerList as $Player ) {
	 
	 //Get stats from default game type
	 $sth = $db->prepare("SELECT * FROM ".OSDB_ALIASES." WHERE default_alias = 1 LIMIT 1");
	 $result = $sth->execute();
	 $row2 = $sth->fetch(PDO::FETCH_ASSOC);
	 
	 $sth = $db->prepare( "SELECT * FROM `".OSDB_STATS."` 
	 WHERE player = '".$Player."' AND alias_id = '".$row2["alias_id"]."' ORDER BY id DESC");
     $result = $sth->execute();
	 $row = $sth->fetch(PDO::FETCH_ASSOC);
	 
	     $ListPlayersData[$c]["slot"] = $c;
		 if ( $row["hide"] == 1 )  $ListPlayersData[$c]["hide"] = 1; else $ListPlayersData[$c]["hide"] = 0;
	     if ( !empty($row["player"]) ) { 
		 $ListPlayersData[$c]["player"] = $row["player"]; 
		 $ListPlayersData[$c]["ip"] = $row["ip"]; 
		 $ListPlayersData[$c]["realm"] = $row["realm"]; 
		 $ListPlayersData[$c]["letter"]   = geoip_country_code_by_addr($GeoIPDatabase, $row["ip"]);
		 $ListPlayersData[$c]["country"]  = geoip_country_name_by_addr($GeoIPDatabase, $row["ip"]);
		 if ($GeoIP == 1 AND empty($ListPlayersData[$c]["letter"]) ) {
		 $ListPlayersData[$c]["letter"] = "blank";
		 $ListPlayersData[$c]["country"] = "Reserved";
		 }
		 
	if ( substr($row["ip"], 0, 7) == "204.14." ) {
	
	  $ListPlayersData[$c]["country"] = "Canada";
	  $ListPlayersData[$c]["letter"]  = "CA";
	
	}
		 
		 $Calculate++; 
		 
		 } 
		 if ( empty($row["player"]) ) { 
		 $ListPlayersData[$c]["no_stats"] = $Player;
		 
		   if ( !empty($Player) ) {
		   $ListPlayersData[$c]["letter"] = "blank";
		   $ListPlayersData[$c]["country"] = "Not ranked";
		   }
		   
		 }
		 else $ListPlayersData[$c]["no_stats"] = "";
		 
		 if ( is_numeric($row["player"]) ) {
		 $ListPlayersData[$c]["no_stats"] = $Player;
		 $ListPlayersData[$c]["player"] = "";
		 $ListPlayersData[$c]["points"] = "";
		 }
		 
		 $ListPlayersData[$c]["level"] = $row["user_level"];
		 $ListPlayersData[$c]["score"] = $row["score"];
		 $ListPlayersData[$c]["games"] = $row["games"];
		 $ListPlayersData[$c]["wins"] = $row["wins"];
		 $ListPlayersData[$c]["losses"] = $row["losses"];
		 $ListPlayersData[$c]["points"] = $row["points"];
	 $c++;
	  
	 }
	 
	 //if ( $Calculate <=1 ) { die( $lang["gl_compare_no"] ); }
	 
	 // class LogCompareLeftTable is 400 px and you hard-code table to be 50% ffs nab
	 // so 1 table is 200px and another too.
	 // You know nothing Michael xD
	 ?>

	 <div class="LogCompareWrapper GameLogSimpleWrapper" id="WinChanceWrapper">
	 <div class="LogCompareLeftTable">
	 <table class="LogCompareTableWidth table table-striped table-hover">
	 <tr>
	  <th width="28"></th>
	  <th width="150"><?=$lang["gl_compare_sentinel"] ?></th>
	  <th><?=$lang["gl_compare_wl"]?></th>
	  <th width="50"><?=$lang["gl_compare_perc"]?></th>
	 </tr>
	 <?php
	 $TotalWins = 0;
	 $TotalLoses = 0;
	 $SentWins = 0;
	 $ScourWins = 0;
	 $SentLoses = 0;
	 $ScourLoses = 0;
	 
	 $SentinelWP = 0;
	 $ScourgeWP = 0;
	 foreach($ListPlayersData as $PlayerData) {
	 
	 if ( $PlayerData["slot"]>5 AND !isset($ScourgeTable) ) {
     $ScourgeTable = 1;
	 ?>
	 </table>
	 </div>
	 <div class="LogCompareRightTable">
	 <table class="table table-striped table-hover">
	 <tr>
	  <th width="28"></th>
	  <th width="230"><?=$lang["gl_compare_scourge"]?></th>
	  <th width="150"><?=$lang["gl_compare_wl"]?></th>
	  <th width="90"><?=$lang["gl_compare_perc"]?></th>
	 </tr>

	 <?php
    } 	 
//Sentinel
    $Notice = "";
	$NoticeTooltip = '<img onMouseout="hidetooltip()" onMouseover="tooltip(\''.$lang["gl_no_stats_data"] .'\', \''.OS_HOME.'img/warn.png\', 200, 32, 32)" src="'.OS_HOME.'img/warn.png" width="16" height="16" class="imgvalign" alt="" />';
	
	if (!empty($PlayerData["player"]) ) $PlayerName = $PlayerData["player"];
	else $PlayerName = "<i>".$PlayerData["no_stats"]."</i>";
	 
	if ($PlayerData["wins"]>=1)   $PlayerData["wins"] = ROUND($PlayerData["wins"], 1); 
	if ($PlayerData["losses"]>=1) $PlayerData["losses"] = ROUND($PlayerData["losses"], 1); 
	
	$CheckPlayerName = strip_tags($PlayerName);
	
	 if ( !isset($ScourgeTable) ) {
	    $SentWins+=$PlayerData["wins"];
		$SentLoses+=$PlayerData["losses"];
		//Why the hell I put this code below -.-
		//if ($PlayerData["wins"]>=1)
		//$SentinelWP.= ROUND(($PlayerData["wins"]/ ($PlayerData["wins"]+$PlayerData["losses"]) )*100, 1); else //$SentinelWP.= 0;

		if ($PlayerData["wins"]>=1) {
		  $TotalSinglePlayerGames = $PlayerData["wins"]+$PlayerData["losses"];
		  if ($TotalSinglePlayerGames>=10) 
		  $Percent = ROUND(($PlayerData["wins"]/ ($PlayerData["wins"]+$PlayerData["losses"]) )*100, 1); 
		  else { 
		  $Percent = 33;
		  $Notice = $NoticeTooltip;
		  }
		}
		else { $Percent = 33;  $Notice = $NoticeTooltip; 	 if (empty($CheckPlayerName)) $Percent = 0; }
		$SentinelWP+=$Percent;

	 } else {
	 //Scourge

	    $ScourWins+=$PlayerData["wins"];
		$ScourLoses+=$PlayerData["losses"];
		//Same - Why the hell I put this code below -.-
		//if ($PlayerData["wins"]>=1)
		//$ScourgeWP.= ROUND(($PlayerData["wins"]/ ($PlayerData["wins"]+$PlayerData["losses"]) )*100, 1); else //$ScourgeWP.= 0;

		if ($PlayerData["wins"]>=1) {
		$TotalSinglePlayerGames = $PlayerData["wins"]+$PlayerData["losses"];
		if ($TotalSinglePlayerGames>=10) 
		$Percent = ROUND(($PlayerData["wins"]/ ($PlayerData["wins"]+$PlayerData["losses"]) )*100, 1); 
		else { 
		  $Percent = 33;
		  $Notice = $NoticeTooltip;
		  }
		}
		else { $Percent = 33;  $Notice = $NoticeTooltip; 	 if (empty($CheckPlayerName)) $Percent = 0; }
		$ScourgeWP+=$Percent;
	 }

	 if ( !empty($PlayerData["no_stats"]) ) {
	    $PlayerData["wins"] = '<span class="no_rank">'.$lang["gl_not_ranked"].'</span>'; 
		$PlayerData["losses"] = "";
	 }
	 
	 ?>
	 <tr>
	  <td class="padLeft"><?=$PlayerData["slot"]?></td>
	  <td class="slot<?=$PlayerData["slot"]?>" width="250">
	  <?php if (!empty($PlayerData["letter"])) OS_ShowUserFlag( $PlayerData["letter"], $PlayerData["country"] );?>
      <?php if ($PlayerData["wins"]<=0 AND ($PlayerData["losses"]<=0) ) { ?>	
      <span class="PlayerListSingle col<?=$PlayerData["slot"]?>"><?=$PlayerName?></span>
      <?php } else { ?>	  
	    <a href="<?=OS_HOME?>?u=<?=strip_tags($PlayerName)?>" target="_blank">
		<span class="PlayerListSingle col<?=$PlayerData["slot"]?>"><?=$PlayerName?></span>
	    <?=OS_IsUserGameAdmin( $PlayerData["level"] )?>
		</a>
	 <?php } ?>
	  <?php if (!empty($PlayerData["realm"])) { ?><div style="font-size:11px;"><?=$PlayerData["realm"]?></div><?php  } ?>
	  </td>
	  <td width="219"><span class="won"><?php if ($PlayerData["hide"] == 0) { ?><?=($PlayerData["wins"])?></span> / <span class="lost"><?=($PlayerData["losses"])?></span> <?=$Notice?> <?php } else echo "hidden"; ?>
	  <?php if (OS_is_admin()) { ?><span style="font-size:10px; float:right;"><?php if ($PlayerData["points"]>=1) { echo number_format($PlayerData["points"],0); ?>pts<?php } ?></span><?php } ?>
	  
	  </td>
	  <td width="68"><?php if ($PlayerData["hide"] == 0) { ?><b><?=$Percent?> %<?php } else echo "-"; ?></b></td>
	 </tr>
	 <?php
	 }
	 ?>
	 </table>
	 </div>
	 </div>
	 
	 <div class="clr"></div>
	 
	 <?php
	 $TOTAL = $SentinelWP+$ScourgeWP;
	 if ($TOTAL>=1) $SentinelWinChanceFixed = ROUND(($SentinelWP/$TOTAL)*100, 1); else $SentinelWinChanceFixed = 0;
	 if ($TOTAL>=1) $ScourgeWinChanceFixed = ROUND(($ScourgeWP/$TOTAL)*100, 1);   else $ScourgeWinChanceFixed  = 0;
	 ?>
	 
	 <table class="LogCompareResults table table-striped table-hover">
	   <tr>
	     <th width="100"><?=$lang["gl_compare_wl_sentinel"]?></th>
		 <th width="200"><?=$lang["gl_compare_sentinel_chance_t"]?></th>
	   </tr>
	   <tr>
	     <td width="100"><span class="won"><?=$SentWins?></span>/<span class="lost"><?=ROUND($SentLoses,1)?></span></td>
		 <td width="200"><?=$lang["gl_compare_sentinel_chance"]?> <b><?=$SentinelWinChanceFixed?>%</b></td>
	   </tr>
	   <tr>
	     <th width="100"><?=$lang["gl_compare_wl_scourge"]?></th>
		 <th width="200"><?=$lang["gl_compare_scourge_chance_t"]?></th>
	   </tr>
	   <tr>
	     <td width="100"><span class="won"><?=$ScourWins?></span>/<span class="lost"><?=ROUND($ScourLoses,1)?></span></td>
		 <td width="200"><?=$lang["gl_compare_scourge_chance"]?> <b><?=$ScourgeWinChanceFixed?>%</b></td>
	   </tr>
	 </table>
   
   <?php
   /*
     if ( isset($_SESSION["level"]) AND $_SESSION["level"]>=9 ) {
	 $TOTAL = $SentinelWP+$ScourgeWP;
	 $SentinelWinChanceFixed = ROUND(($SentinelWP/$TOTAL)*100, 1);
	 $ScourgeWinChanceFixed = ROUND(($ScourgeWP/$TOTAL)*100, 1);
	 
    ?>
	<b>Debug:</b> 
	<div>Sentinel Total: <?=$SentinelWP?></div>
	<div>Scourge Total: <?=$ScourgeWP?></div>
	<div>TOTAL: <?=($SentinelWP+$ScourgeWP)?></div>
	SENT WinChance: <b><?=$SentinelWinChanceFixed?> %</b> | 
	SCOU WinChance: <b><?=$ScourgeWinChanceFixed?> %</b> | 
	<?php
    }
*/	
   ?>
   
   <?php
   
   
   } else if ( isset($_POST["command"]) AND os_is_logged() AND $_SESSION["level"]>=9 ) {
   
     $com = $_POST["command"] = str_replace(array("'", '"'), array(" ", ''), $_POST["command"]);
	 $com = str_replace(array("'", '"'), array(" ", ''), $com);
	 $user = strip_tags(trim($_POST["user"]));
	 $botID = (int) $_POST["botID"];
	 if (isset($_POST["gameID"])) $gameID = (int) $_POST["gameID"]; else $gameID = "";
	 
	 if (isset($_POST["rcon"])) $type = (int) $_POST["rcon"]; else $type = "";
	 
	 if ( $botID <0)  $botID = 1; 
	 $command = "";
	 if ( $com == "mute" )   $command = "!rcon mute ".$_SESSION["username"]." $user";
	 if ( $com == "unmute" ) $command = "!rcon unmute ".$_SESSION["username"]." $user";
	 if ( $com == "kick" )   $command = "!rcon kick ".$_SESSION["username"]." $user";
	 if ( $com == "from" )   $command = "!rcon from";

	$InsertID = -1;
	
	if (!empty($command) ) { $db->insert( OSDB_COMMANDS_QUEUE, array(
	"botid" => $botID,
	"userid" => $_SESSION["user_id"],
	"command" => $command 
     )); 
	 $InsertID = $db->lastInsertId();
	 }
	 ?>
	 <div style="background-color: #000; border: 1px solid #ccc; color: #fff; width: 900px;"> 
	 [BOT] (<?=date(OS_DATE_FORMAT, time() )?>) <?=$command?><br />
	 [BOT] (<?=date(OS_DATE_FORMAT, time() )?>) executed command #<?=$InsertID?>, botID: <?=$botID?>
	 </div>
	 <?php
   }
   else if ( isset($_POST["rcon"]) AND os_is_logged() AND $_SESSION["level"]>=9 AND isset($_POST["gameID"]) ) {
    
	$rcon = safeEscape( trim($_POST["rcon"]) );
	$com = (trim(($_POST["com"])));
	//$com = str_replace('&amp;', '&',$com);
	//$com = convEnt($com);
	//$com = str_replace('&amp;quot;', '"',$com);
	//$com = OS_StrToUTF8($com);
	$gameID = (int) $_POST["gameID"];
	$botID = (int) $_POST["botID"];
	
	 if ( $rcon == 1 ) $command = "!rcon saylobby ".$_SESSION["username"]." $gameID $com";
	 if ( $rcon == 2 ) $command =  "!rcon saygame ".$_SESSION["username"]." $gameID $com";
     if ( $rcon == 3 ) $command =  "!rcon sayteam ".$_SESSION["username"]." $gameID 1 $com";
	 if ( $rcon == 4 ) $command =  "!rcon sayteam ".$_SESSION["username"]." $gameID 2 $com";
	 if ( $rcon == 5 ) $command =  "!rcon from $gameID";
	
	$InsertID = -1;
	
	if (!empty($com) ) { $db->insert( OSDB_COMMANDS_QUEUE, array(
	"botid" => $botID,
	"userid" => $_SESSION["user_id"],
	"command" => $command 
     )); 
	 $InsertID = $db->lastInsertId();
	 }
	?>
	 <div style="background-color: #000; border: 1px solid #ccc; color: #fff; width: 900px;"> 
	 [BOT] (<?=date(OS_DATE_FORMAT, time() )?>) <?=$command?><br />
	 [BOT] (<?=date(OS_DATE_FORMAT, time() )?>) executed command #<?=$InsertID?>, gameID: <?=$gameID?>
	  [<a href="javascript:;" onclick="CloseRcon()">close</a>]
	 </div>	
	<?php
   }
   else if (isset($_POST["status"]) ) {
   	   $sth = $db->prepare("SELECT * FROM ".OSDB_GAMELOG." ORDER BY id DESC LIMIT 1 ");
	   $result = $sth->execute();
	   $row = $sth->fetch(PDO::FETCH_ASSOC);
	   $text = "Bot Online";
	   $lastGameTime = strtotime($row["log_time"]);
	   $botOffline = time() - $lastGameTime;
	   //90 seconds timout - bot OFFLINE.
	   if ( $lastGameTime+90 >= time() ) { $icon = "BotOnline.png"; } else {
	   
	   $icon = "BotOffline.png";
	   $text = "Offline ".( ROUND($botOffline/60,0))." min";
	  if (empty($row["log_time"]) OR strtotime($row["log_time"])=='0000:00:00 00:00' OR $row["log_time"] == 0) $botOffline = "Offline";
	   }
    ?>
	<img <?=ShowToolTip($text, OS_HOME.'img/'.$icon, 140, 32, 32)?> src="<?=OS_HOME?>img/<?=$icon?>" alt="" width="24" height="24" class="imgvalign" />
	<?php
   } 
   
  //CURRENT GAMES
  else if ( isset( $_POST["currentgames"] ) ) {
    global $GameListPatch;
	
	if ( file_exists("inc/geoip/geoip.inc") AND !class_exists("GeoIP")  ) {
	include_once("inc/geoip/geoip.inc");
	$GeoIPDatabase = geoip_open("inc/geoip/GeoIP.dat", GEOIP_STANDARD);
	$GeoIP = 1;
	}
	
	if ( isset($GameListPatch ) AND $GameListPatch  == 1 ) {
	  $sth = $db->prepare( "SELECT * FROM ".OSDB_GAMELIST." WHERE lobby = 1 ORDER BY botid ASC, gamename DESC"  );
	  $result = $sth->execute();
	  $c=0;
	  $LiveGamesData = array();
	  $CurrentPlayers = array();
	  $LivePlayers = array();
	  $TotalPlayers = 0;
	   while ($row = $sth->fetch(PDO::FETCH_ASSOC)) {
	  
	  $LiveGamesData[$c]["botid"]  = ($row["botid"]);
	  $LiveGamesData[$c]["lobby"]  = ($row["lobby"]);
	  $LiveGamesData[$c]["duration"]  = ($row["duration"]);
	  $LiveGamesData[$c]["map_type"]  = ($row["map_type"]);
	  $LiveGamesData[$c]["gamename"]  = ($row["gamename"]);
	  $LiveGamesData[$c]["ownername"]  = ($row["ownername"]);
	  $LiveGamesData[$c]["creatorname"]  = ($row["creatorname"]);

	  $LiveGamesData[$c]["users"]  = ($row["users"]);
	  $LiveGamesData[$c]["players"]  = ($row["players"]);
	  $LiveGamesData[$c]["total"]  = ($row["total"]);
	  $TotalPlayers+=$row["players"];
	  $LiveGamesData[$c]["allplayers"] = explode("#", $row["users"]);
	  
	  $c++;
	  }

	?>
 <table class="table table-striped table-hover">
   <thead>
     <tr>
       <th><?=$lang["slots"]?></th>
       <th><?=$lang["game_name"]?></th>
       <th><?=$lang["players"]?></th>
     </tr>
   </thead>
   <tbody>
   <?php
     $runninggames = 0;
     $players = 0;
     $t = 0;
     foreach ( $LiveGamesData as $LiveGames ) {
       if (!empty($LiveGames["gamename"]) ) {
         $runninggames += 1;
         $players = count($LiveGames["allplayers"]);
         $LiveGames["total"] = $LiveGames["total"];
         if ($LiveGames["total"]<=0) $LiveGames["total"] = 0;
         $t++;
         ?>
         <tr>
           <td width="60"><?=( $players-1 )?> / <?=($LiveGames["total"])?></td>
    	   <td width="200" style="font-size:13px;">
	        <a href="javascript:;" onclick="ToClipboard('gninfo<?=$t?>')"><?=$LiveGames["gamename"]?></a>
	        <input type="hidden" id="gninfo<?=$t?>" value="<?=$LiveGames["gamename"]?>" />
	      </td>
	   <td>
           <?php
	     $c = 1; $s = 0; if(isset($team)) unset($team);
	     for($i = 0; $i < count( $LiveGames["allplayers"] ) -1 ; $i++) {

	       $AllData = explode("#", $LiveGames["allplayers"][$i]);
		   foreach ($AllData as $Data) {
		    
			$PlayersData = explode(",", $Data);
			$s = 0;
			if(!empty($PlayersData[3])) {
			$PlayerIP = $PlayersData[6];
			
	       $letter   = geoip_country_code_by_addr($GeoIPDatabase, $PlayerIP);
	       $country  = geoip_country_name_by_addr($GeoIPDatabase, $PlayerIP);
	       if ($GeoIP == 1 AND empty($letter) ) {
	       $letter= "blank";
	       $country = "Reserved";
	       }
		   
		   	if ( substr($PlayerIP, 0, 7) == "204.14." ) {
		   	$country = "Canada";
		   	$letter  = "CA";
		   	}
			
            if ( $PlayerIP == $_SERVER["REMOTE_ADDR"] OR (os_is_logged() AND strtolower($_SESSION["username"]) == strtolower($username) ) ) {
		    $xstyle = 'background-color: yellow';
	        } else $xstyle = '';
						if(isset($PlayersData[1]) AND $PlayersData[1] == 1 AND !isset($team) ) { ?> | <?php $team = 1; }
			$TooltipData = "<strong>".$PlayersData[3]." (".$PlayersData[5]."ms)</strong><div>".$country."</div><div>".$PlayersData[4]."</div>";
			
			if($PlayersData[2]>=6) $fixslot = 1; else $fixslot = 0;
			?>
			<a <?=ShowToolTip($TooltipData, OS_HOME.'img/flags/'.$letter.'.gif', 210, 21, 15)?> target="_blank" href="<?=OS_HOME?>?u=<?=$PlayersData[3]?>" onclick="hidetooltip();" >
		    <span class="col<?=($PlayersData[2] - $fixslot)?>" style="font-size:11px;<?=$xstyle?>"><?=$PlayersData[3]?></span>
		    </a>
			<?php
			} else echo "-";

		   } 
		   $s = 0;
		   }
	     } ?>
	   </td>
         </tr>
     <?php } } ?>
         <tr>
           <th colspan="3" class="info">
             <?=$runninggames?> <?=strtolower($lang["games"])?>,
             <?=$TotalPlayers?> <?=strtolower($lang["players"])?>.
           </th>
         </tr>
       </tbody>
    </table>
<?php if ( isset($GeoIP) AND $GeoIP == 1) geoip_close($GeoIPDatabase); ?>
<?php } else if ( isset( $_POST["search"] )) {

function HighlightKeyword($str, $search) {
    $occurrences = substr_count(strtolower($str), strtolower($search));
    $newstring = $str;
    $match = array();
 
    for ($i=0;$i<$occurrences;$i++) {
        $match[$i] = stripos($str, $search, $i);
        $match[$i] = substr($str, $match[$i], strlen($search));
        $newstring = str_replace($match[$i], '[#]'.$match[$i].'[@]', strip_tags($newstring));
    }
 
    $newstring = str_replace('[#]', '<b>', $newstring);
    $newstring = str_replace('[@]', '</b>', $newstring);
    return $newstring;
 
}
    
    $search = EscapeStr( trim($_POST["search"]));

	$sth = $db->prepare( "SELECT * FROM `".OSDB_STATS_P."` 
	WHERE player LIKE ('%".$search."%') GROUP BY player ORDER BY user_level DESC, id DESC LIMIT 50");
    $result = $sth->execute();
	?>
	<div class="LiveSearchWrapper">
	<a href="javascript:;" onclick="OS_ResetSearch()" style="float: right;"><img src="<?=OS_HOME?>img/close.png" alt="close" width="16" height="16" class="imgvalign" /></a>
	<?php
	while ($row = $sth->fetch(PDO::FETCH_ASSOC)) {

	$player = HighlightKeyword($row["player"], $search);
	 ?>
	 <div><a href="<?=OS_HOME?>?u=<?=$row["id"]?>"><?=$player?></a></div>
	 <?php
	}
	?>
	</div>
	<?php
  } else if ( isset( $_POST["insult"] )) {
  
    // Nothing do to here!
  
  }
  //PLAYER RATING
  else
  if ( os_is_logged() AND isset($_REQUEST["ratings"]) AND isset($_SESSION["bnet_username"]) ) {
  
  $bl = explode(',',$RatePlayersBlackList);
  if(in_array( strtolower($_SESSION["bnet_username"]), $bl ) )
  die("<span style='color:red'>Rating is disabled for you!</span>");
  
	if ( isset($_POST["player"]) AND isset($_POST["gameid"]) AND isset($_POST["rate"]) AND isset($_POST["slot"]) ) {
	   
	   $Player = trim( strip_tags($_POST["player"]) );
	   $gameid = trim( (int)($_POST["gameid"]) );
	   $rate = trim( (int)($_POST["rate"]) );
	   $slot = trim( (int)($_POST["slot"]) );
	   $voter = trim($_SESSION["bnet_username"]);
	   $comment = trim( strip_tags($_POST["comment"]) );
	   
	   //$CHECK1 = 1;
	   if ($rate <= 4 AND isset($CHECK1) AND !empty($comment) ) {
	     ?>
		 <div style="position:fixed;
     width:400px;
     height:140px;
     z-index:15;
     top:50%;
     left:50%;
     margin:-60px 0 0 -200px;
     background:#fff; border: 8px solid #ccc; padding:20px;" id="ratePl">
		  <div>Enter comment (<b><?=$Player?></b>):</div>
		  <textarea style="width: 350px; height:70px;" id="rateComment" onkeyup="countit('rateComment')" maxlength="255"></textarea>
		  <div>
		  <input class="btn btn-xs btn-primary" type="button" value="Submit" class="menuButtons" onclick="RatePlayer('<?=$Player?>', '<?=$gameid?>', '<?=$rate?>', '<?=$slot?>', 'test' )" /> 
		  <input class="btn btn-xs btn-prmary" type="button" onclick="CloseDiv('ratings-<?=$slot?>')" value="Cancel" class="menuButtons" />
		  <div>max. 255 characters</div>
		  </div>
		 </div>
		 <?php
		 die();
	   }
	   
	   if($rate<=0 OR $rate>5) die("Invalid value!");
	   
	   $sth = $db->prepare(  getGameInfo( $gameid ) );
	   $result = $sth->execute();
	   
	   if ( $sth->rowCount()<=0 ) die();
	   
	   while ($row = $sth->fetch(PDO::FETCH_ASSOC)) {
	      if( strtolower($row["name"]) == strtolower($_SESSION["bnet_username"]) AND strtolower($_SESSION["bnet_username"]) !=  strtolower($Player) ) {
		   $CanRate = 1;
		  }
	   }
	   
	   if(isset($CanRate)) {
	    $sth = $db->prepare("SELECT COUNT(*) FROM ".OSDB_GPR." 
		WHERE player='".$Player."' AND voter = '".$voter."' AND gameid = '".$gameid."'");
		
		$result = $sth->execute();
		$r = $sth->fetch(PDO::FETCH_NUM);
	    $numrows = $r[0];
		
		if( $numrows>=1) { unset($CanRate); die(); }
		
		}
	   
	   if(isset($CanRate)) {
	   
	   	$sth = $db->prepare("INSERT INTO ".OSDB_GPR." (player, voter, gameid, time, rate)
		VALUES('".$Player."', '".$voter."', '".$gameid."', '".time()."', '".$rate."' ) ");
	    $result = $sth->execute();
		
	   	$upd = $db->prepare("UPDATE ".OSDB_STATS_P." SET exp = exp+1 WHERE player = '".$voter."' ");
		$UPDATE = $upd->execute();
		
	    //Get current rate	
	    $sth2 = $db->prepare("SELECT COUNT(rate) as totalvotes, SUM(rate) as totalrate FROM ".OSDB_GPR." 
	    WHERE player = '".$Player."'  AND gameid = '".$gameid."' ");
	    $result2 = $sth2->execute();
	 
	    $rates = $sth2->fetch(PDO::FETCH_ASSOC);
	 
	    if($rates["totalrate"]>=1)
	    $rate = $rates["totalrate"]/$rates["totalvotes"]; 
	    else $rate = 0;		
		
		
		$LeftStars = 5 - $rate;
	   
	    for ($x=1; $x<=$rate; $x++) {
		?><img src="<?=OS_THEME_PATH?>images/star-1.png" width="24" height="24" alt="rating star" /> <?php }
		
		if ( $LeftStars >=1 ) 
	    for ($x=1; $x<=$LeftStars; $x++) {
		?><img src="<?=OS_THEME_PATH?>images/star-0.png" width="24" height="24" alt="rating star" /> <?php }
		
		?>
		( votes: <?=$rates["totalvotes"]?>)
		<?php
	   }
	   
	
	} else die("Invalid form!");
	  
  }
  else { die; }
   
?>