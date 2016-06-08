<?php
if (!isset($website) ) { header('HTTP/1.1 404 Not Found'); die; }

$errors = "";

if ( isset($_SESSION["level"]) AND $_SESSION["level"]>=10 AND isset($_GET["reset_points"]) ) {
   if ( !isset($_GET["rp"]) ) { header("location: ".OS_HOME."adm/"); die; } else {
    $upd = $db->prepare("UPDATE ".OSDB_STATS." SET points = '".(int) $_GET["reset_points"]."' "); 
	$result = $upd->execute(); 
	
	OS_AddLog($_SESSION["username"], "[os_points] ALL - Reset points ( ".(int) $_GET["reset_points"]." ) ");
	
	header("location: ".OS_HOME."adm/?players"); die;
   }
}

//Check admin
if ( os_is_logged() ) {
   $email = $_SESSION["email"];
   $sth = $db->prepare("SELECT * FROM ".OSDB_USERS." WHERE user_email=:user_email ");
   $sth->bindValue(':user_email', $email, PDO::PARAM_STR); 
   $result = $sth->execute();
  // die($email);
   if ( $sth->rowCount()>=1) { 
   $row = $sth->fetch(PDO::FETCH_ASSOC);
   $_SESSION["level"] = $row["user_level"];
   } else {
   header("location: ".OS_HOME."");
   die;
   }
}
if ( !os_is_logged() ) {
header("location: ".OS_HOME."?login");
die;
}

if ( isset($_GET["logout"])  ) {
//os_logout();
header("location: ".OS_HOME."?logout");
die;
}

if ( isset($_GET["optimize_tables"]) ) {
  $opt = $db->prepare("OPTIMIZE TABLE 
  `".OSDB_ADMINS."`,    `".OSDB_BANS."`,          `".OSDB_APPEALS."`,  `".OSDB_REPORTS."`,   `".OSDB_COMMENTS."`,
  `".OSDB_DG."`,        `".OSDB_DP."`,            `".OSDB_DL."`,       `".OSDB_GP."`, 
  `".OSDB_GAMES."`,     `".OSDB_HEROES."`,        `".OSDB_ITEMS."`,    `".OSDB_NEWS."`, 
  `".OSDB_NOTES."`,     `".OSDB_STATS."`,         `".OSDB_USERS."`,    `".OSDB_CUSTOM_FIELDS."`,
  `".OSDB_GUIDES."`,    `".OSDB_GAMELOG."`,       `".OSDB_COMMANDS_QUEUE."`. `".OSDB_GAMESTATUS."`,
  `".OSDB_GAME_INFO."`, `".OSDB_BNET_PM."`,       `".OSDB_ADMIN_LOG."`, `".OSDB_GO."`,
  `".OSDB_ALIASES."`,   `".OSDB_GOALS."`,         `".OSDB_LG_LOGS."` ,  `".OSDB_GPROXY."`                 ");
   $result = $opt->execute();
   $ok = 1;
  if ($ok ) $OptimizedTables = 1;
  
}

if ( isset($_GET["delete_cache"]) AND file_exists("../inc/cache/pdheroes") ) {
      if ($handle = opendir("../inc/cache/pdheroes")) {
       while (false !== ($file = readdir($handle))) {
	      if ($file !="." AND  $file !="index.html" AND $file !=".."  ) {
		  unlink("../inc/cache/pdheroes/".$file);
		 }
	  }
   }
}

if ( isset($_GET["delete_file"]) ) {
   $file = strip_tags( strip_quotes( urldecode($_GET["delete_file"]) ) );
   if ( file_exists("../".$file) ) unlink( "../".$file );
   header('location: '.$website.'adm/?view_cache#files');
}

  if ( isset($_POST["login_"])  ) {
    
	$email    = $_POST["login_email"];
	$password = $_POST["login_password"];
	
	if (!preg_match("/^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,6}$/i", $email)) 
	$errors.="<div>Invalid e-mail or password</div>";
	
	if ( empty($errors)  ) {
	  $sth = $db->prepare("SELECT * FROM ".OSDB_USERS." WHERE user_email = '".$email."' LIMIT 1");
	  $result = $sth->execute();
	  if ( $sth->rowCount()>=1 ) {
	  $row = $sth->fetch(PDO::FETCH_ASSOC);
	  $CheckPW = generate_password($password, $row["password_hash"]);
	  if (!empty($row["code"]) ) $errors.="<div>Account is not activated yet</div>";
	  
	  if ($row["user_password"] == $CheckPW AND empty($errors) ) {
	  $_SESSION["user_id"] = $row["user_id"];
	  $_SESSION["username"] = $row["user_name"];
	  $_SESSION["email"]    = $row["user_email"];
	  $_SESSION["level"]    = $row["user_level"];
	  $_SESSION["can_comment"]    = $row["can_comment"];
	  $_SESSION["logged"]    = time();
	  }
	  
	  } else $errors.="<div>Invalid e-mail or password</div>";
	  
	}
	
  }
  
	$HomeTitle = "Admin | DotA OpenStats v4";

if ( isset( $_GET["posts"]) )     $HomeTitle = "Posts | DotA OpenStats v4";   else
if ( isset( $_GET["bans"]) )      $HomeTitle = "Bans | DotA OpenStats v4";    else
if ( isset( $_GET["admins"]) )    $HomeTitle = "Admins | DotA OpenStats v4";  else
if ( isset( $_GET["safelist"]) )  $HomeTitle = "Safelist | DotA OpenStats v4"; else
if ( isset( $_GET["users"]) )     $HomeTitle = "Users | DotA OpenStats v4";   else
if ( isset( $_GET["games"]) )     $HomeTitle = "Games | DotA OpenStats v4";   else
if ( isset( $_GET["comments"]) )  $HomeTitle = "Comments | DotA OpenStats v4"; else
if ( isset( $_GET["cfg"]) )       $HomeTitle = "Configuration | DotA OpenStats v4";     else
if ( isset( $_GET["notes"]) )     $HomeTitle = "Notes | DotA OpenStats v4";  else
if ( isset( $_GET["ban_reports"]) )  $HomeTitle = "Ban Reports | DotA OpenStats v4"; else
if ( isset( $_GET["ban_appeals"]) )  $HomeTitle = "Ban Appeals | DotA OpenStats v4"; else 
if ( isset( $_GET["about_us"]) )     $HomeTitle = "About Us | DotA OpenStats v4"; else
if ( isset( $_GET["heroes"]) )       $HomeTitle = "Heroes | DotA OpenStats v4"; else
if ( isset( $_GET["items"]) )        $HomeTitle = "Items | DotA OpenStats v4"; else 
if ( isset( $_GET["guides"]) )       $HomeTitle = "Guides | DotA OpenStats v4"; else
if ( isset( $_GET["plugins"]) )      $HomeTitle = "Plugins | DotA OpenStats v4"; else 
if ( isset( $_GET["players"]) )      $HomeTitle = "Ranked Players | DotA OpenStats v4"; else
if ( isset( $_GET["warns"]) )        $HomeTitle = "Warns Players | DotA OpenStats v4"; else
if ( isset( $_GET["live_games"]) )   $HomeTitle = "Live Games | DotA OpenStats v4"; else
if ( isset( $_GET["bnet_pm"]) )      $HomeTitle = "Private Messages BNET | DotA OpenStats v4"; else
if ( isset( $_GET["word_filter"]) )  $HomeTitle = "Word Filter | DotA OpenStats v4"; else
if ( isset( $_GET["ban_email"]) )    $HomeTitle = "Ban Email | DotA OpenStats v4"; else
if ( isset( $_GET["pp"]) )           $HomeTitle = "Penalty Points and Offences | DotA OpenStats v4"; else
if ( isset( $_GET["admin_logs"]) AND $_SESSION["level"]>=10 )  $HomeTitle = "Admin Logs | DotA OpenStats v4"; else
if ( isset( $_GET["remote"]) )       $HomeTitle = "Remote Control | DotA OpenStats v4"; else
if ( isset( $_GET["aliases"]) )      $HomeTitle = "Game Types | DotA OpenStats v4"; else 
if ( isset( $_GET["ban_names"]) )    $HomeTitle = "Ban Names | DotA OpenStats v4"; else
if ( isset( $_GET["gproxy"]) )       $HomeTitle = "GProxy | DotA OpenStats v4";

//Version check

if ( !isset( $_SESSION["v_check"]) ) {
   
   $_SESSION["v_check"] = OS_VERSION;
   
    $v = OS_Curl('http://ohsystem.net/stats/version_check.php?check='.OS_VERSION);
	$os_check = OS_Curl('http://ohsystem.net/stats/version.php');
	
	if ( $os_check != OS_VERSION AND !empty($os_check) ) {
	   $IntroMessage = '<b>An updated version of Dota OpenStats is available.</b><br />';
	   $IntroMessage.= 'You can update to OpenStats <b>'.$os_check."</b><br />";
	   $IntroMessage.= 'Download the package and install it: <br />';
	   $IntroMessage.= '<a target="_blank" class="menuButtons" href="https://github.com/OHSystem/ohsystem/">Download '.$os_check.'</a> <br />';
	   $_SESSION["intro_message"] = $IntroMessage;
	}
	
	if (isset($_SESSION["username"]) AND !isset($_SESSION["adm_logged"]) ) { 
	OS_AddLog($_SESSION["username"], "[os_login] in admin panel");
	$_SESSION["adm_logged"] = time();
	}
	
}
?>