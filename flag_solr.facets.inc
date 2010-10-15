<?php

/**
 * @file
 * Contains the PHP classes implementing our facets.
 *
 * <h2>Overview</h2>
 *
 * There are two kinds of facets:
 *
 * 1. Facets that tell us whether items are flagged (or not flagged) by
 *    the current user.
 *
 * 2. Facets that tell us whether items are flagged (or not flagged) by
 *    any user.
 *
 * The first kind is only applicable for non-global flags, of course. It
 * is implemented by the FlagFacetBySelf class.
 *
 * The second kind is applicable for both global and non-global flags (For
 * example, the admin would want to list all the items that are flagged by
 * anybody, using a "Spam" non-global flag). It is implemented by the
 * FlagFacetByAnybody class.
 *
 * Both PHP classes inherit from FlagFacet.
 */

abstract class FlagFacet {

  var $flag;
  var $by_self; // @todo: remove; not used.

  function __construct($flag_name, $by_self) {
    $this->flag = flag_get_flag($flag_name);
    $this->by_self = $by_self;
  }

  // -----------------------------------------------------------------------
  // Static methods.

  /**
   * Instantiates a facet handler by an ID.
   *
   * @see _flag_solr__id2flag()
   */
  static function by_id($id) {
    if (($extract = _flag_solr__id2flag($id))) {
      list($flag_name, $scope) = $extract;
      if ($scope == 'uid') {
        $facet = new FlagFacetBySelf($flag_name);
      }
      else {
        $facet = new FlagFacetByAnybody($flag_name);
      }
      if ($facet->flag) {
        return $facet;
      }
    }
  }

  static function get_enabled_facets() {
    $facets = array();
    foreach (apachesolr_get_enabled_facets('flag_solr') as $delta => $facet_field) {
      if (($facet = FlagFacet::by_id($delta))) {
        $facets[] = $facet;
      }
    }
    return $facets;
  }

  // -----------------------------------------------------------------------
  // Non-static methods.

  /**
   * Returns the "delta".
   *
   * The delta is the identifier for the facet block.
   */
  abstract function get_delta();

  /**
   * Returns the Solr field used for filtering.
   */
  abstract function get_field();

  /**
   * Returns the facet's definition array.
   */
  abstract function get_definition();

  /**
   * Returns the title for the block.
   *
   * @param $admin
   *   Whether we're shown on the block administration page or to the
   *   end-user.
   */
  function get_block_title($admin) {
    if ($admin) {
      return t('Flag: Filter by !flag-title', array('!flag-title' => $this->get_lc_title()));
    }
    else {
      return t('Filter by !flag-title', array('!flag-title' => $this->get_lc_title()));
    }
  }

  function get_lc_title() {
    return drupal_strtolower($this->flag->get_title());
  }

  /**
   * Returns the "value" of the facet. These are shown inside the facet block.
   *
   * This could be, for example, "Bookmarked" if the field exists and
   * "Not bookmarked" if the field does not exist.
   *
   * @todo: We're at a disadvantage when dealing with Human languages so we
   * should probably delegate this to a theming function.
   *
   * @see get_breadcrumb_value()
   */
  function get_facet_value($exclude) {
    if ($exclude) {
      return t('Not !flag-title', array('!flag-title' => $this->get_lc_title()));
    }
    else {
      return $this->flag->get_title();
    }
  }

  /**
   * Same as get_facet_value() but used for the breadcrumbs and the "Current
   * search" block.
   */
  function get_breadcrumb_value($exclude) {
    return $this->get_facet_value($exclude) . ' (bcmb)';
  }

  /**
   * Returns TRUE if the facet is enabled.
   */
  function is_enabled() {
    $enabled = apachesolr_get_enabled_facets('flag_solr');
    return !empty($enabled[$this->get_delta()]);
  }

  /**
   * Returns TRUE if the facet makes sense with the content-type the query is
   * showing.
   */
  function is_visible($query) {
    // @todo: add a 'content_types' slot to our definition so the following
    // would really do something.
    return apachesolr_block_visibility($query, 'flag_solr', $this->get_delta());
  }

  /**
   * Returns TRUE if the admin has elected to show a filter for non-flagged
   * items as well.
   *
   * (It actually returns either 1 or 0: for compatibility with the radio
   * buttons.)
   */
  function show_missing() {
    $facet_missing = variable_get('apachesolr_facet_missing', array());
    return isset($facet_missing['flag_solr'][$this->get_delta()]) ? $facet_missing['flag_solr'][$this->get_delta()] : 0;
  }

  /**
   * Returns the configuration form for the block.
   */
  function block_form() {
    // Snippet copied from apachesolr_facetcount_form().
    $facet_missing = variable_get('apachesolr_facet_missing', array());
    $form['apachesolr_facet_missing'] = array(
      '#type' => 'radios',
      '#title' => t('Include a facet for missing'),
      '#options' => array(0 => t('No'), 1 => t('Yes')),
      '#description' => t('A facet can be generated corresponding to all documents entirely missing this field.'),
      '#default_value' => $this->show_missing(),
    );
    return $form;
  }

  function block_form_save($edit) {
    // Snippet copied from apachesolr_facetcount_save().
    $facet_missing = variable_get('apachesolr_facet_missing', array());
    $facet_missing['flag_solr'][$this->get_delta()] = (int)$edit['apachesolr_facet_missing'];
    variable_set('apachesolr_facet_missing', $facet_missing);
  }

  /**
   * Returns the block definition. Called by hook_block($op=='list').
   */
  abstract function get_block_definition();

  /**
   * Builds the block. Called by hook_block($op=='view').
   */
  abstract function block($response, $query);

  /**
   * Responds to hook_apachesolr_modify_query().
   *
   * If your facet needs to do some unconventional querying, here is the place
   * to do it.
   */
  function modify_query($query, &$params, $caller) {
  }

}

class FlagFacetBySelf extends FlagFacet {

  function __construct($flag_name) {
    parent::__construct($flag_name, TRUE);
  }

  function get_delta() {
    return 'flag_' . $this->flag->name . '_uid';
  }

  function get_field() {
    return 'im_flag_' . $this->flag->name . '_uid';
  }

  function get_definition() {
    return array(
      'info' =>  $this->get_block_title(TRUE),

      // The ApacheSolr module directly supports 'facet.field' facets. But we
      // have to use a 'facet.query' facet instead (in our hook_modify_query).
      // So since we're doing the querying ourselves, we should make ApacheSolr
      // effectively do nothing by telling it:
      //
      //   'facet_field' => 'flag_some_dummy_field',
      //
      // However, it turns out ApacheSolr has a bug: apachesolr_search_browse()
      // won't display the block unless it can see this field in
      // $response->facet_counts->facet_fields. So we need to give ApacheSolr
      // some real field to work with:
      'facet_field' => 'bs_flag_' . $this->flag->name . '_flagged',
      // (That's the boolean field of the "By Anybody" facets.)

      // 'content_types' => ...,  @todo
    );
  }

  function get_block_definition() {
    // Note that we add DRUPAL_CACHE_PER_USER, because different users see different content.
    // @todo: does it worth to cache this block? I suspect not. And it could be resource-consuming.
    return $this->get_definition() + array('cache' => BLOCK_CACHE_PER_PAGE | BLOCK_CACHE_PER_USER);
  }

  function get_block_title($admin) {
    if ($this->flag->global) {
      return parent::get_block_title($admin);
    }
    if ($admin) {
      return t('Flag: Filter by !flag-title (by current user)', array('!flag-title' => $this->get_lc_title()));
    }
    else {
      return t('Filter by !flag-title (yours)', array('!flag-title' => $this->get_lc_title()));
    }
  }

  /**
   * Builds the block.
   *
   * apachesolr_facet_block() doesn't deal well with 'facet.query' facets,
   * so we provide our own code.
   */
  function block($response, $query) {

    global $user;

    $facet_query = $this->get_field() . ':' . $user->uid;
    $missing_facet_query = '-' . $facet_query;

    // Get rid of PHP notices.
    foreach (array($facet_query, $missing_facet_query) as $q) {
      if (!isset($response->facet_counts->facet_queries->$q)) {
        $response->facet_counts->facet_queries->$q = 0;
      }
    }

    $items = array();

    foreach (array('existing', 'missing') as $mode) {

      if ($mode == 'missing') {
        $count = $response->facet_counts->facet_queries->$missing_facet_query;
        $exclude = TRUE;
      }
      else {
        $count = $response->facet_counts->facet_queries->$facet_query;
        $exclude = FALSE;
      }
      $facet_text = $this->get_facet_value($exclude);

      if ($count && ($mode != 'missing' || $this->show_missing())) {

        // Figure out if we're active. Unfortunately, $query->has_filter() doesn't
        // accept a third, $exclude parameter.
        if (($filter = $query->get_filters($this->get_field()))) {
          $filter = reset($filter); // It's the first.
        }
        $active = $filter && $filter['#exclude'] == $exclude;

        $options = array();
        $new_query = clone $query;
        if ($active) {
          $new_query->remove_filter($this->get_field(), $user->uid);
          $options['query'] = $new_query->get_url_queryvalues();
          $link = theme('apachesolr_unclick_link', $facet_text, $new_query->get_path(), $options);
        }
        else {
          $new_query->add_filter($this->get_field(), $user->uid, $exclude);
          $options['query'] = $new_query->get_url_queryvalues();
          $link = theme('apachesolr_facet_link', $facet_text, $new_query->get_path(), $options, $count, FALSE, $response->response->numFound);
        }
        $items[] = $link;
      }
    }

    if ($items) {
      $output = theme('apachesolr_facet_list', $items, 0, $this->get_delta());
      return array('subject' => $this->get_block_title(FALSE), 'content' => $output);
    }
  }

  /**
   * Responds to hook_apachesolr_modify_query().
   *
   * ApacheSolr handles only 'facet.field' and 'facet.date' facets, but we
   * need to handle 'facet.query' facets ourselves.
   */
  function modify_query($query, &$params, $caller) {
    global $user;
    if ($this->is_visible($query)) {
      $params['facet.query'][] = $this->get_field() . ':' . $user->uid;
      if ($this->show_missing()) {
        $params['facet.query'][] = '-' . $this->get_field() . ':' . $user->uid;
      }
    }
  }

}

class FlagFacetByAnybody extends FlagFacet {

  function __construct($flag_name) {
    parent::__construct($flag_name, FALSE);
  }

  function get_delta() {
    return 'flag_' . $this->flag->name . '_flagged';
  }

  function get_field() {
    return 'bs_flag_' . $this->flag->name . '_flagged';
  }

  function get_definition() {
    return array(
      'info' => $this->get_block_title(TRUE),
      'facet_field' => $this->get_field(),
      // 'content_types' => ...,  @todo
    );
  }

  function get_block_definition() {
    return $this->get_definition() + array('cache' => BLOCK_CACHE_PER_PAGE);
  }

  function get_block_title($admin) {
    if ($this->flag->global) {
      return parent::get_block_title($admin);
    }
    if ($admin) {
      return t('Flag: Filter by !flag-title (by any user)', array('!flag-title' => $this->get_lc_title()));
    }
    else {
      return t('Filter by !flag-title (by any user)', array('!flag-title' => $this->get_lc_title()));
    }
  }

  /**
   * Builds the block.
   *
   * The standard apachesolr_facet_block() can build our block, but it's somewhat
   * "over qualified". For this reason, and for uniformity with FlagFacetBySelf::block(),
   * we provide our own code.
   */
  function block($response, $query) {

    $facet_field = $this->get_field();

    _flag_solr_dsm($response->facet_counts->facet_fields);

    if (!empty($response->facet_counts->facet_fields->$facet_field)) {
      $contains_active = FALSE;
      $items = array();
      foreach ($response->facet_counts->facet_fields->$facet_field as $facet_value => $count) {
        $options = array();
        $exclude = FALSE;

        // Solr sends this back if it's empty.
        if ($facet_value == '_empty_') {
          $exclude = TRUE;
          $facet_value = '[* TO *]';
          $options['html'] = TRUE;
        }

        $facet_text = $this->get_facet_value($exclude);

        $active = $query->has_filter($facet_field, $facet_value);

        $new_query = clone $query;
        if ($active) {
          $contains_active = TRUE;
          $new_query->remove_filter($facet_field, $facet_value);
          $options['query'] = $new_query->get_url_queryvalues();
          $link = theme('apachesolr_unclick_link', $facet_text, $new_query->get_path(), $options);
        }
        else {
          $new_query->add_filter($facet_field, $facet_value, $exclude);
          $options['query'] = $new_query->get_url_queryvalues();
          $link = theme('apachesolr_facet_link', $facet_text, $new_query->get_path(), $options, $count, FALSE, $response->response->numFound);
        }

        if ($count || $active) {
          $items[] = $link;
        }
      }

      // Unless a facet is active only display 2 or more.
      if ($items && ($response->response->numFound > 1 || $contains_active)) {
        $output = theme('apachesolr_facet_list', $items, 0, $this->get_delta());
        return array('subject' => $this->get_block_title(FALSE), 'content' => $output);
      }
    }
  }

}

/**
 * Returns the flag-name, and the facet "scope", corresponding to an ID.
 *
 * ApacheSolr works extensively with various ID strings. This function
 * traces them to their "source". It recognizes the following IDs: block
 * deltas, breadcrumb IDs, and field names.
 *
 * @return array ($flag_name, $scope)
 *   Where $scope is either "flagged" (for FlagFacetByAnybody) or "uid" (for
 *   FlagFacetySelf).
 *
 * @see FlagFacet::by_id()
 */
function _flag_solr__id2flag($id) {
  if (preg_match('/^flag_(.*)_(flagged|uid)$/', $id, $m)) {
    // Delta.
    return array($m[1], $m[2]);
  }
  if (preg_match('/^apachesolr_breadcrumb_(?:bs|im)_flag_(.*)_(flagged|uid)$/', $id, $m)) {
    // Breadcrumbs.
    return array($m[1], $m[2]);
  }
  if (preg_match('/^(?:bs|im)_flag_(.*)_(flagged|uid)$/', $id, $m)) {
    // Fields (theme_apachesolr_breadcrumb_flag()).
    return array($m[1], $m[2]);
  }
}




// -----------------------------------------------------------------------
// Debugging utils.

// @todo: remove.

// A hack to allow Devel's dsm() to print a Apache_Solr_Document.
function __doc_to_obj($document) {
  $obj = (object)array();
  foreach ($document as $k => $v) {
    $obj->$k = $v;
  }
  return $obj;
}

function _flag_solr_dsm($obj) {
  if (function_exists('dsm')) {
    if (get_class($obj) == 'Apache_Solr_Document') {
      $obj = __doc_to_obj($obj);
    }
    return dsm($obj);
  }
}
