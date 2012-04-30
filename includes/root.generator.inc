<?php

/**
 * @file
 * Provides functions for generating subthemes of the Root base theme.
 */

/**
 * Retrieves all themes that can be used as base themes for generating
 * subthemes. Viable base themes are all non-starterkit themes that are based on 'root',
 * as well as 'root' itself.
 */
function root_generator_base_themes() {
  $themes = list_themes();
  $options = array('root' => $themes['root']->info['name']);

  foreach ($themes as $theme) {
    if (isset($theme->info['base theme']) && empty($theme->info['starterkit'])) {
      $base = system_find_base_themes($themes, $theme->name);

      if (isset($base['root'])) {
        $options[$theme->name] = $theme->info['name'];
      }
    }
  }

  return $options;
}

/**
 * @todo
 */
function root_generator_starterkits($base = NULL) {
  $bases = root_generator_base_themes();

  $options = array();
  foreach (list_themes() as $name => $theme) {
    $info = $theme->info;

    if (!empty($info['starterkit']) && isset($info['base theme']) && array_key_exists($info['base theme'], $bases) && ($info['base theme'] == $base || !isset($base))) {
      $options[$name] = $info['name'] . '<div class="description">' . $info['description'] . '</div>';
    }
  }

  return $options;
}

/**
 * @todo
 */
function root_generator_copy_recursive($source, $destination) {
  if (is_dir($source)) {
    if (!file_prepare_directory($destination, FILE_CREATE_DIRECTORY)) {
      return FALSE;
    }

    $directory = dir($source);

    while (FALSE !== ($read = $directory->read())) {
      if ($read != '.' && $read != '..' ) {
        if (!root_generator_copy_recursive($source . '/' . $read, $destination . '/' . $read)) {
          return FALSE;
        }
      }
    }

    $directory->close();
  }
  else {
    file_unmanaged_copy($source, $destination);
  }

  return TRUE;
}

/**
 * @todo
 */
function root_generator_move($source, $destination) {
  if (root_generator_copy_recursive($source, $destination)) {
    file_unmanaged_delete_recursive($source);

    return $destination;
  }

  return FALSE;
}

/**
 * @todo
 */
function root_generator_rewrite_recursive($path, $search, $replace, $rename) {
  if ($path !== ($new = str_replace($search, $rename, $path))) {
    if (!$path = file_unmanaged_move($path, $new, FILE_EXISTS_REPLACE)) {
      return FALSE;
    };
  }

  if (is_dir($path)) {
    $directory = dir($path);

    while (FALSE !== ($read = $directory->read())) {
      if ($read != '.' && $read != '..' ) {
        if (!root_generator_rewrite_recursive($path . '/' . $read, $search, $replace, $rename)) {
          return FALSE;
        }
      }
    }

    $directory->close();
  }
  else {
    root_generator_replace_contents($path, $search, $replace);
  }

  return TRUE;
}

/**
 * @todo
 */
function root_generator_replace_contents($file, $search, $replace) {
  if (is_file($file) && filesize($file) > 0) {
    $before = file_get_contents($file);

    if ($before != ($after = str_replace($search, $replace, $before))) {
      file_unmanaged_save_data($after, $file, FILE_EXISTS_REPLACE);
    }
  }
}

/**
 * @todo
 */
function root_generator_build_info_file($array, $prefix = FALSE) {
  $info = '';

  foreach ($array as $key => $value) {
    if (is_array($value)) {
      $info .= root_generator_build_info_file($value, (!$prefix ? $key : "{$prefix}[{$key}]"));
    }
    else {
      $info .= $prefix ? ("{$prefix}[" . $key .']') : $key;
      $info .= " = '" . str_replace("'", "\'", $value) . "'\n";
    }
  }

  return $info;
}

/**
 * @todo
 */
function root_generator_write_info_file($name, $info, $destination = NULL) {
  $destination = isset($destination) ? $destination : drupal_get_path('theme', $name);

  if (!empty($destination)) {
    return file_unmanaged_save_data(root_generator_build_info_file($info), $destination . '/' . $name . '.info', FILE_EXISTS_REPLACE);
  }

  return FALSE;
}

/**
 * @todo
 */
function root_generator_subtheme_create(&$subtheme) {
  if (is_dir($subtheme->path)) {
    file_unmanaged_delete_recursive($subtheme->path);
  }

  if (!file_prepare_directory($subtheme->path, FILE_CREATE_DIRECTORY)) {
    drupal_set_message(t('Omega Tools could not create the directory %dir.', array('%dir' => $subtheme->path)), 'error');
    watchdog('Omega Tools', t('Omega Tools could not create the directory %dir.', array('%dir' => $subtheme->path)), array(), WATCHDOG_ERROR);

    return FALSE;
  }

  if ($subtheme->starterkit) {
    if (!root_generator_copy_recursive(drupal_get_path('theme', $subtheme->starterkit), $subtheme->path)) {
      $themes = list_themes();

      drupal_set_message(t('Omega Tools could not copy the starterkit %starterkit.', array('%starterkit' => $themes[$subtheme->starterkit]->info['name'])), 'error');
      watchdog('Omega Tools', t('Omega Tools could not copy the starterkit %starterkit.', array('%starterkit' => $themes[$subtheme->starterkit]->info['name'])), array(), WATCHDOG_ERROR);

      return FALSE;
    }

    $subtheme->info = drupal_parse_info_file($subtheme->path . '/' . $subtheme->starterkit . '.info');

    file_unmanaged_delete($subtheme->path . '/' . $subtheme->starterkit . '.info');
  }
  else {
    if (!root_generator_copy_recursive(drupal_get_path('module', 'root_generator') . '/default', $subtheme->path)) {
      drupal_set_message(t('Omega Tools could not copy the default pattern.'), 'error');
      watchdog('Omega Tools', t('Omega Tools could not copy the default pattern.'), array(), WATCHDOG_ERROR);

      return FALSE;
    }

    $base = drupal_parse_info_file(drupal_get_path('theme', $subtheme->base) . '/' . $subtheme->base . '.info');
    $subtheme->info = drupal_parse_info_file($subtheme->path . '/default.pattern');
    $subtheme->info['regions'] = isset($base['regions']) ? $base['regions'] : array();
    $subtheme->info['zones'] = isset($base['zones']) ? $base['zones'] : array();
    $subtheme->info['settings'] = isset($base['settings']) ? $base['settings'] : array();

    file_unmanaged_delete($subtheme->path . '/default.pattern');
  }

  unset($subtheme->info['starterkit'], $subtheme->info['hidden'], $subtheme->info['locked'], $subtheme->info['project'], $subtheme->info['datestamp']);

  $subtheme->info['name'] = $subtheme->name;
  $subtheme->info['description'] = '';
  $subtheme->info['base theme'] = $subtheme->base;
  $subtheme->info['engine'] = 'phptemplate';
  $subtheme->info['core'] = DRUPAL_CORE_COMPATIBILITY;
  $subtheme->info['version'] = '1.x';
  $subtheme->info['screenshot'] = 'screenshot.png';
  $subtheme->info['regions'] = array_merge(_root_generator_core_regions(), $subtheme->info['regions']);
  $subtheme->info['zones'] = array_merge(_root_generator_default_zones(), $subtheme->info['zones']);

  root_generator_write_info_file($subtheme->machine_name, $subtheme->info, $subtheme->path);
  root_generator_rewrite_recursive($subtheme->path, 'YOURTHEME', $subtheme->machine_name, str_replace('_', '-', $subtheme->machine_name));

  drupal_set_message(t('You have successfully created the theme %name.', array('%name' => $subtheme->name)));

  return TRUE;
}

/**
 * @todo
 */
function root_generator_subtheme_process($subtheme) {
  drupal_theme_rebuild();
  system_rebuild_theme_data();

  if ($subtheme->status) {
    theme_enable(array($subtheme->machine_name));

    if ($subtheme->default) {
      variable_set('theme_default', $subtheme->machine_name);
      drupal_set_message(t('%name is now the default theme.', array('%name' => $subtheme->name)));
    }
  }
  else {
    theme_disable(array($subtheme->machine_name));
  }
}

/**
 * @todo
 */
function root_generator_sites() {
  $sites = &drupal_static(__FUNCTION__);

  if (!isset($sites)) {
    $sites = array();

    if (file_exists(DRUPAL_ROOT . '/sites/sites.php')) {
      include(DRUPAL_ROOT . '/sites/sites.php');
    }

    $sites = array('all') + array_values($sites);
  }

  return $sites;
}

/**
 * @todo
 */
function root_generator_sites_options() {
  $options = array();

  if ($sites = root_generator_sites()) {
    $options += array_combine($sites, $sites);
  }

  $options['all'] = t('Default destination (all)');

  return $options;
}

/**
 * @todo
 */
function root_generator_write_archive($source, $name, $destination = NULL) {
  $destination = isset($destination) ? $destination : 'temporary://root';

  if (!file_prepare_directory($destination, FILE_CREATE_DIRECTORY)) {
    drupal_set_message(t('Root could not create the directory %dir', array('%dir' => $destination)), 'error');
    watchdog('Root', t('Root could not create the directory %dir', array('%dir' => $destination)), array(), WATCHDOG_ERROR);

    return FALSE;
  }

  $destination = $destination . '/' . $name . '.tar';
  $current = getcwd();

  chdir(drupal_realpath(dirname($source)));

  $archiver = new Archive_Tar(drupal_realpath($destination));
  $archiver->create(basename(rtrim($source, '/')));

  chdir($current);

  return $destination;
}

/**
 * @todo
 */
function root_generator_revert_theme_settings($theme) {
  $themes = list_themes();

  variable_del('theme_' . $theme . '_settings');

  drupal_theme_rebuild();
  system_rebuild_theme_data();

  watchdog('theme', t('Theme settings for %theme reset to default values.', array('%theme' => $themes[$theme]->info['name'])));
  drupal_set_message(t('The theme settings for %theme have been purged from the database and are now being loaded from the .info file of this theme.', array('%theme' => $themes[$theme]->info['name'])));
}

/**
 * @todo
 */
function _root_generator_theme_exists($theme) {
  $themes = list_themes();

  return isset($themes[$theme]);
}

/**
 * @todo
 */
function _root_generator_validate_theme_name($element, &$form_state) {
  system_rebuild_theme_data();

  if (!preg_match('/^[a-z][a-z0-9_]*$/', $element['#value'])) {
    form_error($element, t('The theme name is invalid. It may only contain lowercase numbers, letters and underscores and must start with a letter.'));
  }
  else if (_root_generator_theme_exists($element['#value'])) {
    form_error($element, t('A theme with that name already exists. The machine-readable name must be unique.'));
  }
}

/**
 * @todo
 */
function _root_generator_validate_theme_settings($element, &$form_state) {
  $settings = drupal_parse_info_format($element['#value']);

  foreach ($settings as $key => $value) {
    if ($key !== 'settings') {
      form_error($element, t('You may only submit theme settings with this form.'));
    }
  }
}

/**
 * @todo
 */
function _root_generator_transform_theme_name($name) {
  $name = preg_replace('/^[^a-z]+/', '', strtolower($name));
  $name = preg_replace('/[^a-z0-9_]+/', '_', $name);
  $name = str_replace('__', '_', $name);
  $name = trim($name, '_');

  return $name;
}

/**
 * @todo
 */
function _root_generator_theme_access($theme) {
  if ($theme && user_access('administer themes')) {
    $themes = list_themes();

    if (!isset($themes[$theme->name]) || _root_generator_is_editable($theme->name)) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
 * @todo
 */
function _root_generator_is_starterkit($theme) {
  return array_key_exists($theme, root_generator_starterkits());
}

/**
 * @todo
 */
function _root_generator_is_editable($theme) {
  $themes = list_themes();

  if (!in_array($theme, array('omega', 'alpha')) && strpos(drupal_get_path('theme', $theme), 'themes') !== 0) {
    if (isset($themes[$theme]->info['base theme'])) {
      $base = system_find_base_themes($themes, $theme);

      if (!empty($base) && isset($base['alpha']) && empty($themes[$theme]->info['locked'])) {
        return TRUE;
      }
    }
  }

  return FALSE;
}
