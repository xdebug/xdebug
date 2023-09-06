<?php
if ($argc <= 1) {
	die("Usage: {$argv[0]} <version> <<'from_master'>>");
}

$xdebugRepo    = '/home/derick/dev/php/xdebug-xdebug';
$xdebugOrgRepo = '/home/derick/dev/php/xdebug-xdebug.org';

$url = "https://bugs.xdebug.org/api/rest/";
$project_id = 1;

$release_version = $argv[1];

$from_master = false;
if ( $argc == 3 ) {
	$from_master = ( $argv[2] == 'from_master' );
}

$stability = 'stable';

if ( preg_match( '/beta|alpha|RC|rc/', $release_version ) )
{
	$stability = 'beta';
}

echo "Releasing for $release_version ($stability)\n\n";

/* Check whether there is a Mantis version */
$project = file_get_contents( "{$url}/projects/{$project_id}" );
$p_info = json_decode( $project );

function findVersion( $p_info, $release_version )
{
	$versions = $p_info->projects[0]->versions;

	foreach ( $versions as $key => $version )
	{
		if ( $version->name === $release_version )
		{
			return $version;
		}
	}

	return false;
}

function getAllFixedIssuesForVersion( $version_id )
{
	global $url;

	echo "Fetching issues: ";
	$page      = 1;
	$page_size = 100;
	$found     = [];

	do {
		$pageUrl = "{$url}/issues?page_size={$page_size}&page={$page}";
		echo " {$page}";

		$r = json_decode( file_get_contents( $pageUrl ) );

		foreach ( $r->issues as $issue )
		{
			if ( isset( $issue->fixed_in_version ) && $issue->fixed_in_version->id == $version_id )
			{
				$found[$issue->id] = $issue->summary;
			}
		}
		$page++;
	} while( count( $r->issues ) > 0 );

	ksort( $found );

	echo "\n\n";
	return $found;
}

function writePackageXMLInclusion( $release_version, $issues )
{
	global $stability;

	$date = date( 'Y-m-d' );
	$time = date( 'H:i:s' );
	$long_date = date( 'D, M d, Y' );

	$bugs = '';
	foreach ( $issues as $id => $description )
	{
		$description = htmlspecialchars( $description );
		$bugs .= "  - Fixed issue #{$id}: {$description}\n";
	}

$xml = <<<ENDXML
 <date>{$date}</date>
 <time>{$time}</time>
 <version>
  <release>{$release_version}</release>
  <api>{$release_version}</api>
 </version>
 <stability>
  <release>{$stability}</release>
  <api>{$stability}</api>
 </stability>
 <license uri="https://xdebug.org/license/1.03" filesource="LICENSE">Xdebug-1.03</license>
 <notes>
{$long_date} - Xdebug {$release_version}

= Fixed bugs:

{$bugs}
 </notes>

ENDXML;

	file_put_contents( '/tmp/package.xml', $xml );
	echo "package.xml snippet is at /tmp/package.xml\n";
}


/*
function createVersion( $project_id, $release_version )
{
	global $url;

	echo "Creating version {$release_version}\n";

	$c = curl_init();
	curl_setopt($c, CURLOPT_URL, "{$url}/projects/{$project_id}/versions");
	curl_setopt($c, CURLOPT_POST, true);
	curl_setopt($c, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($c, CURLOPT_HTTPHEADER, [
		'Content-Type: application/json',
		'Authorization: ' . getenv( 'MANTIS_TOKEN' ),
	] );

	curl_setopt($c, CURLOPT_POSTFIELDS, json_encode(
		[
			'name' => $release_version,
			'released' => false,
			'obsolete' => false,
			'timestamp' => date( 'Y-m-d' ),
		]
	));

	$result = curl_exec( $c );
	curl_close( $c );
	print_r( $result );
}
*/

function updateGIT( bool $from_master )
{
	`git checkout master`;
	`git pull origin master`;

	if ( !$from_master )
	{
		`git checkout xdebug_3_2`;
		`git pull origin xdebug_3_2`;
	}
}

function updateTemplateRC( $release_version )
{
	$version = strtr( $release_version, '.', ',' );
	$file = file_get_contents( 'template.rc' );
	$file = preg_replace( '/#define VERSIONDESC.*/', "#define VERSIONDESC $version,1", $file );
	$file = preg_replace( '/#define VERSIONSTR.*/', "#define VERSIONSTR  \"{$release_version}\"", $file );
	$file = preg_replace( '/2002-202. Derick Rethans/', '2002-' . date('Y') . ' Derick Rethans', $file );

	file_put_contents( 'template.rc', $file );
}

function updatePhpXdebugH( $release_version )
{
	$file = file_get_contents( 'php_xdebug.h' );
	$file = preg_replace( '/#define XDEBUG_VERSION.*/', "#define XDEBUG_VERSION    \"{$release_version}\"", $file );
	$file = preg_replace( '/\(c\) 2002-202./', '(c) 2002-' . date('Y'), $file );

	file_put_contents( 'php_xdebug.h', $file );
}

function rebuild()
{
	echo "Rebuilding Xdebug binary:\n";
	`~/bin/rebuild.sh`;
}

function updateIniFileInDocs()
{
	global $xdebugRepo, $xdebugOrgRepo;

	$cwd = getcwd();
	chdir( $xdebugOrgRepo );
	`php html/docs/convert.php > {$xdebugRepo}/xdebug.ini`;

	chdir( $cwd );
}

function peclPackage()
{
	/*
	echo "Running 'pecl package':\n";
	pcntl_exec( "/usr/local/php/8.0dev/bin/pecl package" );
	*/
	echo "pecl package\n";
}

function installPeclPackage( $release_version )
{
	/*
	echo "Installing new PECL package:\n";
	pcntl_exec( "/usr/local/php/8.0dev/bin/pecl install -f xdebug-{$release_version}.tgz" );
	*/
	echo "pecl install -f xdebug-{$release_version}.tgz 2>&1 >/dev/null && php -v\n";
}

function showGitCommands( $release_version )
{
	echo "git commit package.xml template.rc php_xdebug.h xdebug.ini RELEASE_PROCESS.rst -m \"Go with {$release_version}\"\n";
	echo "~/bin/tag-sign.sh {$release_version}\n";
	echo "git push origin {$release_version}\n";

	echo "\n";
}

function createUpdatesSection( $release_version, $issues )
{
	global $xdebugOrgRepo;

	$version = strtr( $release_version, '.', '_' );
	$date = date( 'Y-m-d' );

	$bugs = '';
	foreach ( $issues as $id => $description )
	{
		$description = htmlspecialchars( $description );
		$bugs .= "<dd>Fixed <?= bug({$id}); ?>: {$description}</dd>\n";
	}

$xml = <<<ENDXML
<dt><a name='x_{$version}'></a>[{$date}] &mdash; Xdebug {$release_version}</dt>

<dd><h3>Fixed bugs</h3></dd>

{$bugs}
<hr/>


ENDXML;

	file_put_contents( '/tmp/updates.php', $xml );
	echo "{$xdebugOrgRepo}/views/home/updates.php snippet is at /tmp/update.php\n";
}

function createXdebugVersionPhp( $release_version )
{
	global $xdebugOrgRepo;

$xml = <<<ENDXML
    public const NOT_SUPPORTED_BEFORE = '3.1';
    public const LATEST_VERSION = '{$release_version}';
    public const LATEST_WINDOWS_VERSION = '{$release_version}';

    private const VERSIONS =  [
        '7.0' => [ 'src' => '2.8.1',                        ],
        '7.1' => [ 'src' => '2.9.8',       'win' => '2.9.8' ],
        '7.2' => [ 'src' => '3.1.6',       'win' => '3.1.6' ],
        '7.3' => [ 'src' => '3.1.6',       'win' => '3.1.6' ],
        '7.4' => [ 'src' => '3.1.6',       'win' => '3.1.6' ],
        '8.0' => [ 'src' => '{$release_version}',       'win' => '{$release_version}' ],
        '8.1' => [ 'src' => '{$release_version}',       'win' => '{$release_version}' ],
        '8.2' => [ 'src' => '{$release_version}',       'win' => '{$release_version}' ],
        '8.3' => [ 'src' => '{$release_version}',       'win' => '{$release_version}' ],
    ];

ENDXML;

	file_put_contents( '/tmp/XdebugVersion.php', $xml );
	echo "{$xdebugOrgRepo}/src/XdebugVersion.php snippet is at /tmp/XdebugVersion.php\n";
}

function updateDocsController( $release_version )
{
	global $xdebugOrgRepo;
	global $stability;

	$file = file_get_contents( "{$xdebugOrgRepo}/src/Controller/DocsController.php" );

	if ( $stability == 'stable' )
	{
		$file = preg_replace( "@'\[KW:last_release_version\]', '3\..\..'@", "'[KW:last_release_version]', '{$release_version}'", $file );
	}

	file_put_contents( "{$xdebugOrgRepo}/src/Controller/DocsController.php", $file );
}

function createNewsTemplate( $release_version )
{
	global $xdebugOrgRepo;

	$version = strtr( $release_version, '.', '_' );
	$date = date( 'Y-m-d' );

$text = <<<ENDTXT
Xdebug {$release_version} is out!
<p>
This is a bug fix release that
</p>

<p>
The full list of changes can be found on the
<a href="https://xdebug.org/updates#x_{$version}">updates</a> page.
</p>

<p>
The source code can be found on the
<a href="https://xdebug.org/download#releases">downloads</a> page, and as
usual, Xdebug is installable through PECL.
</p>

<p>
If you find a bug, please file a report at Xdebug's
<a href="https://bugs.xdebug.org">Issue Tracker</a>.
</p>

<!--
<p>
This release also contains a contribution by: <i></i> — Thanks!
</p>
-->
ENDTXT;

	file_put_contents( "{$xdebugOrgRepo}/data/news/{$date}.txt", $text );

	$cwd = getcwd();
	chdir( $xdebugOrgRepo );
	`git add data/news/{$date}.txt`;

	chdir( $cwd );
}

$r = findVersion( $p_info, $release_version );
if ( $r === false )
{
	/*
	createVersion( $project_id, $release_version );
	$r = findVersion( $p_info, $release_version );
	*/
	die("The version {$release_version} does not exist");
}

echo "Existing version {$release_version}\n";

echo "Checking date: ";
$version_date = preg_match( '/^(20.*)T/', $r->timestamp, $m );
if ( ! $version_date || $m[1] != date('Y-m-d') )
{
	echo "\nUpdate version with right date:\nhttps://bugs.xdebug.org/manage_proj_ver_edit_page.php?version_id={$r->id}\n";
}
else
{
	echo "OK\n";
}

$issues = getAllFixedIssuesForVersion( $r->id );
if ( count( $issues) == 0 )
{
	die("There are no issues for version {$release_version}");
}

updateGIT( $from_master );
updateTemplateRC( $release_version );
updatePhpXdebugH( $release_version );
rebuild();
updateIniFileInDocs();

echo "\nRun the following commands:\n\n";

peclPackage();
installPeclPackage( $release_version );
showGitCommands( $release_version );

writePackageXMLInclusion( $release_version, $issues );
createUpdatesSection( $release_version, $issues );

if ( $stability === 'stable' )
{
	createXdebugVersionPhp( $release_version );
}
updateDocsController( $release_version );
createNewsTemplate( $release_version );

echo "\nSet version 'release' to 'true':\nhttps://bugs.xdebug.org/manage_proj_ver_edit_page.php?version_id={$r->id}\n";
