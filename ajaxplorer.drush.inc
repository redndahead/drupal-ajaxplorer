<?php

/**
 * @file
 *   Example drush command.
 *
 *   To run this *fun* command, execute `sudo drush --include=./examples mmas`
 *   from within your drush directory.
 *
 *   See `drush topic docs-commands` for more information about command authoring.
 *
 *   You can copy this file to any of the following
 *     1. A .drush folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. /usr/share/drush/commands (configurable)
 *     4. In an arbitrary folder specified with the --include option.
 *     5. Drupal's sites/all/drush folder.
 */

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * See `drush topic docs-commands` for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function ajaxplorer_drush_command() {
  $items = array();

  // The 'ajaxplorer-set-aliases' command
  $items['ajaxplorer-set-aliases'] = array(
    'description' => "Set's the apache aliases so if someone self hosts the self hosted site will show up.",
    'examples' => array(
      'drush asa' => 'Rebuild all the aliases.',
    ),
    'aliases' => array('asa'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL, // No bootstrap at all.
  );

  return $items;
}



/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'. This hook is optional. If a command
 * does not implement this hook, the command's description is used instead.
 *
 * This hook is also used to look up help metadata, such as help
 * category title and summary.  See the comments below for a description.
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function ajaxplorer_drush_help($section) {
  switch ($section) {
    case 'drush:ajaxplorer-set-aliases':
      return dt("This command will recreate all the apache aliases.");
    // The 'title' meta item is used to name a group of
    // commands in `drush help`.  If a title is not defined,
    // the default is "All commands in ___", with the
    // specific name of the commandfile (e.g. sandwich).
    // Command files with less than four commands will
    // be placed in the "Other commands" section, _unless_
    // they define a title.  It is therefore preferable
    // to not define a title unless the file defines a lot
    // of commands.
    case 'meta:ajaxplorer:title':
      return dt("Ajaxplorer commands");
    // The 'summary' meta item is displayed in `drush help --filter`,
    // and is used to give a general idea what the commands in this
    // command file do, and what they have in common.
    case 'meta:ajaxplorer:summary':
      return dt("Rebuilds apache aliases for self hosters.");
  }
}

/**
 * Implementation of drush_hook_COMMAND_validate().
 *
 * The validate command should exit with
 * `return drush_set_error(...)` to stop execution of
 * the command.  In practice, calling drush_set_error
 * OR returning FALSE is sufficient.  See drush.api.php
 * for more details.
 */
function drush_ajaxplorer_set_aliases_validate() {
  return TRUE;
}

/**
 * Example drush command callback. This is where the action takes place.
 *
 * The function name should be same as command name but with dashes turned to
 * underscores and 'drush_commandfile_' prepended, where 'commandfile' is
 * taken from the file 'commandfile.drush.inc', which in this case is 'sandwich'.
 * Note also that a simplification step is also done in instances where
 * the commandfile name is the same as the beginning of the command name,
 * "drush_example_example_foo" is simplified to just "drush_example_foo".
 * To also implement a hook that is called before your command, implement
 * "drush_hook_pre_example_foo".  For a list of all available hooks for a
 * given command, run drush in --debug mode.
 *
 * If for some reason you do not want your hook function to be named
 * after your command, you may define a 'callback' item in your command
 * object that specifies the exact name of the function that should be
 * called.  However, the specified callback function must still begin
 * with "drush_commandfile_" (e.g. 'callback' => "drush_example_foo_execute")
 * if you want that all hook functions are still called (e.g.
 * drush_example_pre_foo_execute, and so on).
 *
 * In this function, all of Drupal's API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 * @see drush_invoke()
 * @see drush.api.php
 */
function drush_ajaxplorer_set_aliases() {
  // Here is where we do all our processing.
  $aliases_path = '/var/www/html/httpd_conf/aliases';
  $restrictions_path = '/var/www/html/httpd_conf/restrictions';
  $user_directories = '/var/www/html/ajaxplorer/data/personal';
  $aliases = glob($aliases_path . '/*.conf');
  $aliases = array_flip($aliases);
  $restrictions = glob($restrictions_path . '/*.conf');
  $restrictions = array_flip($restrictions);
  $newconf = array();
  $query = "SELECT users.name FROM users LEFT JOIN users_roles on users.uid = users_roles.uid LEFT JOIN role on role.rid = users_roles.rid WHERE role.name = 'self hoster'";
  $result = db_query($query);
  while ($item = db_fetch_array($result)) {
    if (isset($aliases[$aliases_path . '/' . $item['name'] . '.conf'])) {
      unset($aliases[$aliases_path . '/' . $item['name'] . '.conf']);
      if (isset($restrictions[$restrictions_path . '/' . $item['name'] . '.conf'])) {
        unset($restrictions[$restrictions_path . '/' . $item['name'] . '.conf']);
      }

    }
    else {
      $newconf[] = $item['name'];
    }
  }
  
  $reload = FALSE;
  if (!empty($aliases)) {
    $reload = TRUE;
    foreach($aliases as $filepath => $fileid) {
      unlink($filepath);
    }
  }

  if (!empty($restrictions)) {
    $reload = TRUE;
    foreach($restrictions as $filepath => $fileid) {
      unlink($filepath);
    }
  }


  if (!empty($newconf)) {
    $reload = TRUE;
    foreach($newconf as $name) {
      // Create apache alias if user is using ssh.
      $handle = fopen($aliases_path . '/' . $name . '.conf', 'w');
      fwrite($handle, 'alias /' . $name . ' ' . $user_directories . '/' . $name);
      fclose($handle);

      // Create apache restriction if user is using ssh.
      $handle = fopen($restrictions_path . '/' . $name . '.conf', 'w');
      fwrite($handle, '<Directory "' . $user_directories . '/' . $name . '">' . "\n");
      fwrite($handle, 'php_admin_value open_basedir "' . $user_directories . '/' . $name . '"' . "\n");
      fwrite($handle, '</Directory>');
      fclose($handle);
    }
  }

  if (!empty($reload)) {
    drush_print('Reloading apache');
    shell_exec('/etc/init.d/httpd restart');
  }
}

