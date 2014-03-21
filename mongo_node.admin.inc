<?php

/**
 * @file
 * Admin interface for entity types, entity bundles and content
 */

/**
 * Form constructor for creating entity types.
 *
 * @see mongo_node_type_type_create_form_validate()
 * @see mongo_node_type_create_form_submit()
 */
function mongo_node_type_create_form() {
  $form = array();

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Entity type'),
    '#description' => t('The human readable name of the entity.'),
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#required' => FALSE,
    '#machine_name' => array(
      'exists' => '_mongo_node_type_exists',
      'source' => array('label'),
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form validation handler for mongo_node_type_create_form().
 *
 * @see mongo_node_type_create_form_submit()
 */
function mongo_node_type_create_form_validate($form, $form_state) {
  $set = mongo_node_settings();
  $machine_name = $form_state['values']['name'];

  if (isset($set[$machine_name])) {
    form_set_error('name', t('Machine name @machine_name already exists', array('@machine_name' => $machine_name)));
    return FALSE;
  }
  return TRUE;
}

/**
 * Form submission handler for mongo_node_type_create_form().
 *
 * @see mongo_node_type_create_form_validate()
 */
function mongo_node_type_create_form_submit($form, $form_state) {
  $label = $form_state['values']['label'];
  $machine_name = $form_state['values']['name'];

  $set = mongo_node_settings();
  $set += mongo_node_type_default_settings($label, $machine_name);

  // @todo - redirect to entity type page
  mongo_node_settings_save($set);
}

/**
 * Form constructor for editing entity types.
 *
 * @see mongo_node_type_edit_form_submit()
 */
function mongo_node_type_edit_form($form, $form_state, $entity_type) {
  $set = mongo_node_settings();
  $form = array();

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Entity type'),
    '#default_value' => $set[$entity_type]['label'],
    '#description' => t('The human readable name of the entity.'),
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#required' => FALSE,
    '#default_value' => $entity_type,
    '#machine_name' => array(
      'exists' => '_mongo_node_type_exists',
      'source' => array('label'),
    ),
  );

  // Send entity type.
  $form['entity_type'] = array(
    '#type' => 'value',
    '#value' => $entity_type,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form submission handler for mongo_node_type_edit_form().
 */
function mongo_node_type_edit_form_submit($form, $form_state) {
  $label = $form_state['values']['label'];
  $machine_name = $form_state['values']['name'];
  $old_entity_type = $form_state['values']['entity_type'];

  $set = mongo_node_settings();

  if ($machine_name != $old_entity_type) {
    $set += mongo_node_type_default_settings($label, $machine_name);
    $set[$machine_name] = $set[$old_entity_type];
    unset($set[$old_entity_type]);

    // Update existing mongo entities with new entity type.
    _mongo_node_update_type($old_entity_type, $machine_name);
  }
  else {
    $set[$machine_name]['label'] = $label;
  }

  // Save settings.
  mongo_node_settings_save($set);
  return TRUE;
}

/**
 * Form constructor for deleting entity types.
 *
 * @param string $entity_type
 *   Entity type
 *
 * @see mongo_node_type_delete_form_submit()
 */
function mongo_node_type_delete_form($form, $form_state, $entity_type) {
  $form = array();
  // Send entity type.
  $form['entity_type'] = array(
    '#type' => 'value',
    '#value' => $entity_type,
  );
  $path = '/admin/structure/mongo-node';

  $extist_entity_content = '';
  $collection = mongodb_collection('fields_current', $entity_type);
  $count = $collection->count(array('_type' => $entity_type));
  if ($count) {
    $extist_entity_content = t('%type is used by %count piece of content on your site. If you remove this entity type, you will not be able to edit the %type content and it may not display correctly.', array('%type' => $entity_type, '%count' => $count));
    $extist_entity_content .= '<br/><br/>';
  }
  return confirm_form($form, t('Delete entity type'), $path, $extist_entity_content . 'This action cannot be undone', t('Delete'), t('Cancel'), 'mongo_node_type_delete');
}

/**
 * Form submission handler for mongo_node_type_delete_form().
 */
function mongo_node_type_delete_form_submit($form, $form_state) {
  $set = mongo_node_settings();

  // Remove entity type.
  $entity_type = $form_state['values']['entity_type'];
  unset($set[$entity_type]);

  // Drop collection that stores entities for entity type.
  $collection = mongodb_collection('fields_current', $entity_type);
  $collection->drop();

  // Drop collection that stores entity types autoincrement ids.
  $collection = mongodb_collection($entity_type . '_ids');
  $collection->drop();

  mongo_node_settings_save($set);

  // Redirect to entity type admin.
  drupal_goto('admin/structure/mongo-node');
}

/**
 * Entity type existance check - Helper function.
 */
function _mongo_node_type_exists($entity_type) {
  $set = mongo_node_settings();
  if (isset($set[$entity_type])) {
    return TRUE;
  }

  return FALSE;
}


/**
 * Form constructor for the entity bundle creation.
 *
 * @param string $entity_type
 *   Entity type
 *
 * @see mongo_node_bundle_create_form_validate()
 * @see mongo_node_bundle_create_form_submit()
 */
function mongo_node_bundle_create_form($form, $form_state, $entity_type) {
  $form = array();

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Entity type'),
    '#description' => t('The human readable name of the entity.'),
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#required' => FALSE,
    '#machine_name' => array(
      'exists' => '_mongo_node_bundle_exists',
      'source' => array('label'),
    ),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Describe this bundle.'),
  );

  $form['entity_type'] = array(
    '#type' => 'value',
    '#value' => $entity_type,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form validation handler for mongo_node_bundle_create_form().
 *
 * @see mongo_node_bundle_create_form_submit()
 */
function mongo_node_bundle_create_form_validate($form, $form_state) {
  if (empty($form_state['values']['name'])) {
    form_set_error('name', t('Machine name @machine_name already exists', array('@machine_name' => $machine_name)));
    return FALSE;
  }

  $machine_name = $form_state['values']['name'];
  $entity_type = $form_state['values']['entity_type'];

  $set = mongo_node_settings();

  if (isset($set[$entity_type]['bundles'][$machine_name])) {
    form_set_error('name', t('Machine name @machine_name already exists', array('@machine_name' => $machine_name)));
    return FALSE;
  }
  return TRUE;
}

/**
 * Form submission handler for mongo_node_bundle_create_form().
 *
 * @see mongo_node_bundle_create_form_validate()
 */
function mongo_node_bundle_create_form_submit($form, $form_state) {
  $entity_type = $form_state['values']['entity_type'];

  $label = $form_state['values']['label'];
  $machine_name = $form_state['values']['name'];
  $description = $form_state['values']['description'];

  $set = mongo_node_settings();
  $set[$entity_type]['bundles'] += mongo_node_bundle_default_settings($label, $machine_name, $description);

  // @todo - redirect to bundle type page.
  mongo_node_settings_save($set);
}

/**
 * Form constructor for entity bundle editing.
 *
 * @see mongo_node_bundle_edit_form_submit()
 */
function mongo_node_bundle_edit_form($form, $form_state, $entity_type, $bundle) {

  $set = mongo_node_settings();

  $form = array();

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Entity type'),
    '#default_value' => $set[$entity_type]['bundles'][$bundle]['label'],
    '#description' => t('The human readable name of the entity.'),
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#required' => FALSE,
    '#default_value' => $bundle,
    '#machine_name' => array(
      'exists' => '_mongo_node_bundle_exists',
      'source' => array('label'),
    ),
  );

  $description = isset($set[$entity_type]['bundles'][$bundle]['description']) ? $set[$entity_type]['bundles'][$bundle]['description'] : '';
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $description,
    '#description' => t('Describe this bundle.'),
  );

  // Save entity type and bundle.
  $form['bundle_type'] = array(
    '#type' => 'value',
    '#value' => array(
      'entity_type' => $entity_type,
      'bundle' => $bundle,
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form submission handler for mongo_node_bundle_edit_form().
 */
function mongo_node_bundle_edit_form_submit($form, $form_state) {

  $label = $form_state['values']['label'];
  $machine_name = $form_state['values']['name'];
  $entity_type = $form_state['values']['bundle_type']['entity_type'];
  $old_bundle_type = $form_state['values']['bundle_type']['bundle'];
  $description = $form_state['values']['description'];

  $set = mongo_node_settings();

  if ($machine_name != $old_bundle_type) {
    $set[$entity_type]['bundles'] += mongo_node_bundle_default_settings($label, $machine_name, $description);
    $set[$entity_type]['bundles'][$machine_name] = $set[$entity_type]['bundles'][$old_bundle_type];
    unset($set[$entity_type]['bundles'][$old_bundle_type]);

    // Update existing mongo entities with new bundle.
    //_mongo_node_update_bundle($entity_type, $old_bundle_type, $machine_name);
  }
  else {
    $set[$entity_type]['bundles'][$machine_name]['label'] = $label;
    $set[$entity_type]['bundles'][$machine_name]['description'] = $description;
  }

  // Save settings.
  mongo_node_settings_save($set);
  return TRUE;
}

/**
 * Helper function - Rename a mongo collection.
 *
 * @param string $old_entity_type
 *   Source entity_type
 * @param string $new_entity_type
 *   Target entity_type
 */
function _mongo_node_rename_collection($old_entity_type, $new_entity_type) {
  $mongodb = mongodb();

  // Get connection and database name by forcing call to __toString().
  $m = $mongodb->connection;
  $database = sprintf('%s', $mongodb);

  $admin_db = $m->admin;

  $res = $admin_db->command(array(
    "renameCollection" => $database . ".fields_current." . $old_entity_type,
    "to" => $database . ".fields_current." . $new_entity_type,
  ));

  $res = $admin_db->command(array(
    "renameCollection" => $database . "." . $old_entity_type . "_ids",
    "to" => $database . "." . $new_entity_type . "_ids",
  ));
}

/**
 * Helper function update entities type.
 *
 * @param string $old_entity_type
 *   Source entity type
 * @param string $entity_type
 *   Target entity type
 */
function _mongo_node_update_type($old_entity_type, $entity_type) {
  // Update collection names.
  _mongo_node_rename_collection($old_entity_type, $entity_type);

  // Update bundle name in mongo entities.
  $collection = mongodb_collection('fields_current', $entity_type);
  $result = $collection->update(
    array('_type' => $old_entity_type),
    array('$set' => array('_type' => $entity_type, 'type' => $entity_type))
  );
}

/**
 * Helper function update entities bundle.
 *
 * @param string $entity_type
 *   Entity type that changed a bundle name
 * @param string $old_bundle
 *   Source bundle
 * @param string $bundle
 *   Target bundle
 */
function _mongo_node_update_bundle($entity_type, $old_bundle, $bundle) {

  // Update bundle name in ids collection.
  $collection = mongodb_collection($entity_type . '_ids');

  $result = $collection->update(
    array('bundle' => $old_bundle),
    array('$set' => array('bundle' => $bundle))
  );

  // Update bundle name in mongo entities.
  $collection = mongodb_collection('fields_current', $entity_type);
  $result = $collection->update(
    array('_bundle' => $old_bundle, '_type' => $entity_type),
    array('$set' => array('_bundle' => $bundle, 'type' => $bundle))
  );
}

/**
 * Helper function if machine name exists.
 *
 * @param string $bundle
 *   Bundle to check existance
 */
function _mongo_node_bundle_exists($bundle) {
  $entity_type = arg(3);

  $set = mongo_node_settings();
  if (isset($set[$entity_type]['bundles'][$bundle])) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Form constructor for entity bundle delete.
 *
 * @param string $entity_type
 *   Entity type that will have a bundle deleted
 * @param string $bundle
 *   Bundle name that will be deleted
 *
 * @see mongo_node_bundle_delete_form_submit()
 */
function mongo_node_bundle_delete_form($form, $form_state, $entity_type, $bundle) {
  $form = array();

  $form['entity_type'] = array(
    '#type' => 'value',
    '#value' => $entity_type,
  );
  $form['bundle'] = array(
    '#type' => 'value',
    '#value' => $bundle,
  );

  $path = '/admin/structure/mongo-node/' . $entity_type;

  $extist_bundle_content = '';
  $collection = mongodb_collection('fields_current', $entity_type);
  $count = $collection->count(array('_type' => $entity_type, '_bundle' => $bundle));
  if ($count) {
    $extist_bundle_content = t('%bundle is used by %count piece of content on your site. If you remove this bundle, you will not be able to edit the %bundle content and it may not display correctly.', array('%bundle' => $bundle, '%count' => $count));
    $extist_bundle_content .= '<br/><br/>';
  }

  return confirm_form($form, t('Delete entity bundle'), $path, $extist_bundle_content . 'This action cannot be undone', t('Delete'), t('Cancel'), 'mongo_node_bundle_delete');
}

/**
 * Form submission handler for mongo_node_bundle_delete_form().
 */
function mongo_node_bundle_delete_form_submit($form, $form_state) {
  $set = mongo_node_settings();

  $entity_type = $form_state['values']['entity_type'];
  $bundle = $form_state['values']['bundle'];

  // Remove entity type.
  unset($set[$entity_type]['bundles'][$bundle]);

  mongo_node_settings_save($set);

  // Redirect to entity type admin.
  drupal_goto('admin/structure/mongo-node/' . $entity_type);
}

/**
 * Show all defined mongo entities to administer.
 */
function mongo_node_entities() {
  $set = mongo_node_settings();
  $base_path = 'admin/structure/mongo-node';

  $header = array(t('Name'), array(
    'data' => t('Operations'),
    'colspan' => 3,
    ),
  );

  foreach ($set as $entity_type => $settings) {
    $rows[] = array(
      array('data' => l($settings['label'], $base_path . '/' . $entity_type)),
      array('data' => l(t('edit'), $base_path . '/' . $entity_type . '/edit')),
      array('data' => l(t('delete'), $base_path . '/' . $entity_type . '/delete')),
    );
  }

  $output = theme('table', array(
    'rows' => $rows,
    'header' => $header,
    'empty' => t('No entities created yet.'),
    )
  );
  return $output;
}

/**
 * Show for a certain entity type all its bundles.
 */
function mongo_node_bundles($entity_type) {
  $set = mongo_node_settings();
  $base_path = 'admin/structure/mongo-node/' . $entity_type . '/manage';

  $header = array(t('Name'), array(
    'data' => t('Operations'),
    'colspan' => 4,
    ),
  );

  $rows = array();

  foreach ($set[$entity_type]['bundles'] as $bundle => $bundle_settings) {
    $rows[] = array(
      array('data' => l($bundle_settings['label'], $base_path . '/' . $bundle)),
      array('data' => l(t('edit'), $base_path . '/' . $bundle . '/edit')),
      array('data' => l(t('manage fields'), $base_path . '/' . $bundle . '/fields')),
      array('data' => l(t('manage display'), $base_path . '/' . $bundle . '/display')),
      array('data' => l(t('delete'), $base_path . '/' . $bundle . '/delete')),
    );
  }

  $output = theme('table', array(
    'rows' => $rows,
    'header' => $header,
    'empty' => t('No entities created yet.'),
    )
  );

  return $output;
}

/**
 * Menu callback: content administration.
 *
 * @param string $entity_type
 *   A mongo entity type.
 *
 * @return array
 *   An array as expected by drupal_render().
 */
function mongo_node_admin_content($form, $form_state, $entity_type) {
  $settings = mongo_node_settings();

  // Send entity type.
  $form['entity_type'] = array(
    '#type' => 'value',
    '#value' => $entity_type,
  );

  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Show only items where'),
  );

  $filters = mongo_node_filters($entity_type);
  $applied_filters = isset($_SESSION[$entity_type . '_filters']) ? $_SESSION[$entity_type . '_filters'] : array();

  foreach ($filters as $f_type_name => $f_type) {
    $form['filters'][$f_type_name] = array('#type' => 'select', '#title' => $f_type_name);
    foreach ($f_type as $f_name => $f_val) {
      $form['filters'][$f_type_name]['#options'][$f_name] = $f_val['label'];
    }
  }

  $items = array();
  $remaining_filters = array();
  foreach ($applied_filters as $app_filter) {
    $items[] = t('where %f_name is %f_val', array('%f_name' => $app_filter['#type'], '%f_val' => $app_filter['#value']));
    $conditions = $filters[$app_filter['#type']][$app_filter['#value']]['filters'];
    foreach ($conditions as $condition) {
      $remaining_filters[$condition['#type']]['#callback'] = $condition['#callback'];
      $remaining_filters[$condition['#type']]['#value'][] = $condition['#value'];
    }
  }

  $form['filters']['#description'] = theme('item_list', array('items' => $items));

  if (empty($applied_filters)) {
    $form['filters']['filter'] = array(
      '#type' => 'submit',
      '#value' => 'Filter',
      '#submit' => array('mongo_node_filters_submit'),
    );
  }
  else {
    $form['filters']['refine'] = array(
      '#type' => 'submit',
      '#value' => 'Refine',
      '#submit' => array('mongo_node_filters_submit'),
    );
    $form['filters']['undo'] = array(
      '#type' => 'submit',
      '#value' => 'Undo',
      '#submit' => array('mongo_node_filters_submit'),
    );
    $form['filters']['reset'] = array(
      '#type' => 'submit',
      '#value' => 'Reset',
      '#submit' => array('mongo_node_filters_submit'),
    );
  }

  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
  );

  $operations = mongo_node_operations();
  foreach ($operations as $op => $op_set) {
    $options[$op] = $op_set['label'];
  }
  $form['options']['operation'] = array(
    '#type' => 'select',
    '#options' => $options,
  );

  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#validate' => array('mongo_node_operation_validate'),
    '#submit' => array('mongo_node_operation_submit'),
  );

  $query = new EntityFieldQuery();

  $query->entityCondition('entity_type', $entity_type)
    ->propertyOrderBy('created', 'DESC')
    ->pager(10);

  // Actually apply filters.
  foreach ($remaining_filters as $r_name => $r_values) {
    $query->{$r_values['#callback']}($r_name, $r_values['#value'], 'IN');
  }

  $result = $query->execute();

  $languages = language_list();
  $destination = drupal_get_destination();
  $header = array(
    'title' => array('data' => t('Title'), 'field' => 'title'),
    'type' => t('Type'),
    'author' => t('Author'),
    'status' => t('Status'),
    'changed' => t('Updated'),
    'language' => t('Language'),
    'operations' => t('Operations'),
  );
  $options = array();

  if (isset($result[$entity_type])) {
    $ids = array_keys($result[$entity_type]);
    $entities = entity_load($entity_type, $ids);

    foreach ($result[$entity_type] as $id => $efq_entity) {
      $entity = entity_load($entity_type, array($id));
      $entity = reset($entity);

      $entity_uri = entity_uri($entity_type, $entity);

      $options[$id] = array(
        'title' => array(
          'data' => array(
            '#type' => 'link',
            '#title' => $entity->title,
            '#href' => $entity_uri['path'],
          ),
        ),
        'type' => check_plain($settings[$entity_type]['bundles'][$entity->type]['label']),
        'author' => theme('username', array('account' => $entity)),
        'status' => $entity->status ? t('published') : t('not published'),
        'changed' => format_date($entity->changed, 'short'),
        'language' => ($entity->language == LANGUAGE_NONE) ? t('Language neutral') : $languages[$entity->language]->name,
      );

      $operations = array();
      $operations[] = l(t('edit'), $entity_uri['path'] . '/edit', array('query' => $destination));
      $operations[] = l(t('delete'), $entity_uri['path'] . '/delete', array('query' => $destination));

      $options[$id]['operations'] = theme('item_list', array('items' => $operations, 'attributes' => array('class' => 'inline')));
    }
  }

  $form['entities'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No content available'),
  );

  $form['pager'] = array(
    '#theme' => 'pager',
  );

  return $form;
}

/**
 * Content administration page operations validate function.
 */
function mongo_node_operation_validate($form, &$form_state) {
  if (!is_array($form_state['values']['entities']) || !count(array_filter($form_state['values']['entities']))) {
    form_set_error('', t('No items selected.'));
  }
}

/**
 * Content administration page operations submit function.
 */
function mongo_node_operation_submit($form, &$form_state) {
  $operations = mongo_node_operations();
  $operation = $operations[$form_state['values']['operation']];

  $entities = array_filter($form_state['values']['entities']);
  $entity_type = $form_state['values']['entity_type'];
  if ($function = $operation['callback']) {
    // Add in callback arguments if present.
    if (isset($operation['callback arguments'])) {
      $args = array(
        $entity_type,
        $entities,
        $operation['callback arguments'],
      );
    }
    else {
      $args = array($entity_type, $entities);
    }
    call_user_func_array($function, $args);
    cache_clear_all();
  }
}

/**
 * Operations for content administration page.
 */
function mongo_node_operations() {
  $operations = array(
    'publish' => array(
      'label' => t('Publish selected content'),
      'callback' => 'mongo_node_mass_update',
      'callback arguments' => array('updates' => array('status' => 1)),
    ),
    'unpublish' => array(
      'label' => t('Unpublish selected content'),
      'callback' => 'mongo_node_mass_update',
      'callback arguments' => array('updates' => array('status' => 0)),
    ),
    'delete' => array(
      'label' => t('Delete selected content'),
      'callback' => 'mongo_node_mass_delete',
    ),
  );
  return $operations;
}

/**
 * Callbacks for operation select.
 */
function mongo_node_mass_update($entity_type, $entities, $args) {
  foreach ($entities as $entity_id) {
    $entity = entity_get_controller($entity_type)->load(array($entity_id));
    $entity = reset($entity);
    // Skip if not updates.
    if (!isset($args['updates'])) {
      continue;
    }
    foreach ($args['updates'] as $p_name => $p_val) {
      $entity->$p_name = $p_val;
    }
    entity_save($entity_type, $entity);
  }
}

/**
 * Callbacks for operation select.
 */
function mongo_node_mass_delete($entity_type, $entities) {
  entity_delete_multiple($entity_type, $entities);
}

/**
 * List mongo entity administration filters that can be applied.
 */
function mongo_node_filters($entity_type) {
  $filters = array(
    'status' => array(
      'any' => array(
        'label' => 'Any',
      ),
      'published' => array(
        'label' => 'Published',
        'filters' => array(
          array(
            '#type' => 'status',
            '#value' => 1,
            '#callback' => 'propertyCondition',
          ),
        ),
      ),
      'not_published' => array(
        'label' => 'Not published',
        'filters' => array(
          array(
            '#type' => 'status',
            '#value' => 0,
            '#callback' => 'propertyCondition',
          ),
        ),
      ),
    ),
    'type' => array(
      'any' => array(
        'label' => 'Any',
      ),
    ),
  );

  $set = mongo_node_settings();
  $bundles = $set[$entity_type]['bundles'];
  foreach ($bundles as $bundle_name => $bundle_settings) {
    $filters['type'][$bundle_name] = array(
      'label' => $bundle_settings['label'],
      'filters' => array(
        array(
          '#type' => 'bundle',
          '#value' => $bundle_name,
          '#callback' => 'entityCondition',
        ),
      ),
    );
  }
  return $filters;
}

/**
 * Mongo entity filter submit handler.
 *
 * It is used by the filter buttons in mongo_node_admin_content().
 *
 * @see mongo_node_admin_content()
 */
function mongo_node_filters_submit($form, &$form_state) {
  $entity_type = $form_state['values']['entity_type'];
  $filters = mongo_node_filters($entity_type);

  $op = strtolower($form_state['input']['op']);

  if ($op == 'reset') {
    // Remove all filters.
    unset($_SESSION[$entity_type . '_filters']);
  }
  elseif ($op == 'undo') {
    // Remove last filter.
    array_pop($_SESSION[$entity_type . '_filters']);
  }
  else {
    // Apply filters.
    foreach ($filters as $f_type => $f_val) {
      if (!isset($form_state['values'][$f_type]) || $form_state['values'][$f_type] == 'any') {
        continue;
      }
      $filter = $form_state['values'][$f_type];
      if (!isset($_SESSION[$entity_type . '_filters'])) {
        $_SESSION[$entity_type . '_filters'] = array();
      }
      $exists = FALSE;
      foreach ($_SESSION[$entity_type . '_filters'] as &$app_filter) {
        if ($app_filter['#type'] == $f_type && $app_filter['#value'] == $filter) {
          $exists = TRUE;
          break;
        }
      }
      // Skip filter if already applied.
      if ($exists) {
        continue;
      }
      // Add filter to session.
      $_SESSION[$entity_type . '_filters'][] = array(
        '#type' => $f_type,
        '#value' => $filter,
      );
    }
  }
}

/**
 * Page callback: Displays all entity bundle add links.
 *
 * Redirects if only one bundle found
 *
 * @see mongo_node_menu()
 */
function mongo_node_add_page() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);
  if (count($content) == 1) {
    $item = array_shift($content);
    drupal_goto($item['href']);
  }

  return theme('admin_block_content', array('content' => $content));
}

/**
 * Returns a mongo entity submission form.
 *
 * @param string $entity_type
 *   A mongo entity type.
 * @param string $bundle
 *   A mongo entity bundle.
 *
 * @return array
 *   A submission form.
 */
function mongo_node_add($entity_type, $bundle) {
  global $user;
  $set = mongo_node_settings();

  $entity = entity_create($entity_type, array(
      'uid' => $user->uid,
      'name' => (isset($user->name) ? $user->name : ''),
      'type' => $bundle,
      'language' => LANGUAGE_NONE,
    )
  );
  $form_id = $entity_type . '_' . $bundle . '_mongo_node_form';
  $output = drupal_get_form($form_id, $entity, $entity_type);

  return $output;
}

/**
 * Generate the mongo entity add/edit form array.
 *
 * @param MongoNode $entity
 *   A mongo entity object.
 * @param string $entity_type
 *   A mongo entity type.
 *
 * @return array
 *   An array as expected by drupal_render().
 */
function mongo_node_form($form, &$form_state, $entity, $entity_type) {
  $form = array();

  $form_state['entity_type'] = $entity_type;
  if (!isset($form_state[$entity_type])) {
    $form_state[$entity_type] = $entity;
  }
  else {
    $entity = $form_state[$entity_type];
  }

  // Get title label name from properties if exists
  static $settings = array();
  if(empty($settings)) {
    $settings = mongo_node_settings();
  }
  $title = isset($settings[$entity_type]['properties']['title']['label']) ? $settings[$entity_type]['properties']['title']['label'] : t('Title');

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => $title,
    '#required' => TRUE,
    '#default_value' => isset($entity->title) ? $entity->title : '',
  );

  $form['#attributes']['class'][] = 'mongo-node-form';
  if (!empty($entity->type)) {
    // TODO -- add entity type with bundle.
    $form['#attributes']['class'][] = 'mongo-node-' . $entity->type . '-form';
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
    '#submit' => array('mongo_node_form_submit'),
  );

  $form['#validate'][] = 'mongo_node_form_validate';
  field_attach_form($entity_type, $entity, $form, $form_state);

  return $form;
}

/**
 * Mongo entity add/edit validate function.
 */
function mongo_node_form_validate(&$form, &$form_state) {
  $entity_type = $form_state['entity_type'];
  $entity = $form_state[$entity_type];
  field_attach_form_validate($entity_type, $entity, $form, $form_state);
}

/**
 * Form submit handler for mongo_node_form().
 */
function mongo_node_form_submit(&$form, &$form_state) {
  $entity_type = $form_state['entity_type'];
  $entity = entity_ui_controller($entity_type)->entityFormSubmitBuildEntity($form, $form_state);
  $insert = empty($entity->mid);
  entity_save($entity_type, $entity);

  $info = entity_get_info($entity_type);
  $bundle_key = $info['bundle keys']['bundle'];
  $bundle_label = $info['bundles'][$entity->{$bundle_key}]['label'];

  if ($insert) {
    drupal_set_message(t('@label %title has been created.',
        array(
          '@label' => $bundle_label,
          '%title' => $entity->title,
        )
      )
    );
  }
  else {
    drupal_set_message(t('@label %title has been updated.',
        array(
          '@label' => $bundle_label,
          '%title' => $entity->title,
        )
      )
    );
  }

  $uri = entity_uri($entity_type, $entity);
  $form_state['redirect'] = drupal_get_path_alias($uri['path']);
}

/**
 * Menu callback; Generate an array for rendering the given entity.
 *
 * @param string $entity_type
 *   A mongo entity type.
 * @param MongoNode $entity
 *   A mongo entity object.
 *
 * @return array
 *   An array as expected by drupal_render().
 */
function mongo_node_page_view($entity_type, $entity, $langcode = NULL) {
  if (!isset($langcode)) {
    $langcode = $GLOBALS['language_content']->language;
  }

  $controller = entity_get_controller($entity_type);

  $title = $entity->title;
  drupal_set_title($title);

  // Allow modules to change the view mode.
  $view_mode = 'full';
  $context = array(
    'entity_type' => $entity_type,
    'entity' => $entity,
    'langcode' => $langcode,
  );
  drupal_alter('entity_view_mode', $view_mode, $context);

  $content = array();
  list($id, ,) = entity_extract_ids($entity_type, $entity);

  $content = $controller->view(array($id => $entity), $view_mode);

  return $content;
}

/**
 * Menu callback; presents the mongo entity editing form.
 *
 * @param string $entity_type
 *   A mongo entity type.
 * @param MongoNode $entity
 *   A mongo entity object.
 *
 * @return array
 *   An array as expected by drupal_render().
 */
function mongo_node_page_edit($entity_type, $entity) {
  $title = $entity->title;
  drupal_set_title($title);

  $content = array();
  $content = drupal_get_form('mongo_node_form', $entity, $entity_type);

  return $content;
}

/**
 * Menu callback -- ask for confirmation of mongo entity deletion.
 *
 * @param string $entity_type
 *   A mongo entity type.
 * @param MongoNode $entity
 *   A mongo entity object.
 *
 * @return array
 *   A confirm form.
 */
function mongo_node_page_delete($form, $form_state, $entity_type, $entity) {
  $form['#entity'] = $entity;
  $uri = entity_uri($entity_type, $entity);

  return confirm_form($form,
    t('Are you sure you want to delete %title', array('%title' => $entity->title)),
    $uri['path'],
    t('This action cannot be undone'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Mongo entity delete submit handler for mongo_node_page_delete().
 */
function mongo_node_page_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $entity = $form['#entity'];
    $entity->delete();

    $entity_type = $entity->entityType();
    $info = entity_get_info($entity_type);
    $bundle_key = $info['bundle keys']['bundle'];
    $bundle_label = $info['bundles'][$entity->{$bundle_key}]['label'];

    drupal_set_message(t('@label %title deleted',
      array(
        '@label' => $bundle_label,
        '%title' => $entity->title)
      )
    );
  }

  $form_state['redirect'] = '<front>';
}
