<?php

/**
 * Проверка установки фильтра на странице курса валют
 *
 * @return bool $filter - Наличие применения фильтра
 */
function custom_exchange_rates_filter() {

    $filter = FALSE;
    if (isset($_GET['currency']) || isset($_GET['date']))
        $filter = TRUE;

    return $filter;

}


/**
 * Получение заголовков для запроса и таблицы
 *
 * @return array $header - Массив заголовков
 */
function custom_exchange_rates_get_headers() {

    $header = array(
        array('data' => 'Valute ID', 'field' => 'currency.currency_valute_id'),
        array('data' => 'Num Code', 'field' => 'currency.currency_num_code'),
        array('data' => 'Char Code', 'field' => 'currency.currency_char_code'),
        array('data' => 'Nominal', 'field' => 'currency.currency_nominal'),
        array('data' => 'Name', 'field' => 'currency.currency_name'),
        array('data' => 'Value', 'field' => 'rate.currency_value'),
    );

    return $header;

}


/**
 * Получение данных для страницы курса валют
 *
 * @param array $header - Список заголовков для запроса
 * @param bool $pager - Переменная, указывающая на необходимость постраничного деления информации
 *
 * @return array $exchange_rates - Массив объектов курса валют для отображения на странице. Ключ элемента массива - currency_id. Объект курса валют содержит следующие данные:
 * * int currency_id - идентификатор валюты
 * * int currency_valute_id - Valute ID
 * * string currency_num_code - Num Code
 * * string currency_char_code - Char Code
 * * string currency_nominal - Nominal
 * * string currency_name - название валюты
 * * float currency_value - значение валюты
 */
function custom_exchange_rates_get_data($header, $pager = TRUE) {

    if ($pager)
        $query = db_select('custom_exchange_rates_currency', 'currency')->extend('TableSort')->orderByHeader($header)->extend('PagerDefault')->limit(50);
    else
        $query = db_select('custom_exchange_rates_currency', 'currency')->extend('TableSort')->orderByHeader($header);

    $query->innerJoin('custom_exchange_rates_rate', 'rate', 'currency.currency_id = rate.currency_id');
    $query->innerJoin('custom_exchange_rates_date', 'date', 'rate.date_id = date.date_id');

    $last_update_timestamp = custom_exchange_rates_get_last_update_timestamp();

    if (isset($_GET['date']))
        $query->condition('date.date_value', $_GET['date']);
    else
        $query->condition('date.date_value', $last_update_timestamp);

    if (isset($_GET['currency']))
        $query->condition('currency.currency_id', $_GET['currency']);

    $query->fields('currency');
    $query->addField('rate', 'currency_value');

    $exchange_rates = $query->execute()->fetchAll();

    return $exchange_rates;

}


/**
 * Страница курса валют
 *
 * @return string - HTML страницы
 * @throws Exception
 */
function custom_exchange_rates_page() {

    // Заголовки
    $header = custom_exchange_rates_get_headers();

    // Получаем данные согласно GET параметрам
    $rates = custom_exchange_rates_get_data($header);

    // Формируем данные для таблицы
    $rows = array();
    foreach ($rates as $rate) {

        $rows[] = array(
            $rate->currency_valute_id,
            $rate->currency_num_code,
            $rate->currency_char_code,
            $rate->currency_nominal,
            $rate->currency_name,
            $rate->currency_value,
        );

    }

    $pager = theme('pager');

    $filter_form = drupal_get_form('custom_exchange_rates_filter_form');
    $filter_form = render($filter_form);

    global $pager_total_items;
    $total_text = isset($pager_total_items[0]) ? '<p>Всего ' . $pager_total_items[0] . ' ' . russian_plural($pager_total_items[0], 'запись', 'записи', 'записей') . '</p>' : NULL;

    $update_link = '<div class="link update_link">' . l('Обновить курс валют', 'admin/custom_exchange_rates/update') . '</div>';

    $params = drupal_get_query_parameters();

    $report_link = '<div class="link report_link">' . l('Сформировать отчет', 'admin/custom_exchange_rates/json', array( 'query' => $params)) . '</div>';

    return	$update_link .
        $report_link .
        $filter_form .
        $total_text .
        theme('table', array('header' => $header, 'rows' => $rows, 'empty' => 'Записей не найдено')) .
        $pager;

}


/**
 * Форма фильтрации данных на странице
 *
 * @param array $form - Массив формы
 * @param array $form_state - Текущее состояние формы
 *
 * @return array $form - Массив формы
 */
function custom_exchange_rates_filter_form($form, &$form_state) {

    $form['#attributes']['class'][] = 'custom_exchange_rates-filter-form';

    $form['table_prefix'] = array(
        '#markup' => '<table><tr>',
    );

    $last_update_timetamp = custom_exchange_rates_get_last_update_timestamp();

    $default_date_timestamp = isset($_GET['date']) ? $_GET['date'] : $last_update_timetamp;

    $form['date'] = array(
        '#type' => 'date_popup',
        '#title' => 'Дата обновления',
        '#default_value' => date("Y-m-d", $default_date_timestamp),
        '#date_format' => 'Y-m-d',
        '#date_increment' => 10,
        '#date_year_range' => '-3:0',
        '#prefix' => '<td>',
        '#suffix' => '</td>',
        '#pre_render' => array('custom_exchange_rates_date_prerender'),
    );

    $currencys = custom_exchange_rates_get_currency_names();

    $form['currency'] = array(
        '#type' => 'select',
        '#title' => 'Валюта',
        '#options' => $currencys,
        '#default_value' => isset($_GET['currency']) ? $_GET['currency'] : NULL,
        '#empty_option' => 'Все',
        '#prefix' => '<td>',
        '#suffix' => '</td>',
    );

    $filter = custom_exchange_rates_filter();

    $form['actions'] = array(
        '#prefix' => '<td><div class="fgood-form-actions">',
        '#suffix' => '</div></td>',
    );

    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Применить',
    );

    if ($filter)
        $form['actions']['reset'] = array(
            '#markup' => l('Сбросить', current_path()),
        );

    $form['table_suffix'] = array(
        '#markup' => '</tr></table>',
    );

    return $form;

}


/**
 * Предобработка поля "Даты"
 *
 * @param array $element - массив полей элемента формы
 *
 * @return array $element - массив полей элемента формы
 */
function custom_exchange_rates_date_prerender($element) {

    $element['date']['#title'] = $element['#title'];
    $element['date']['#description'] = '&nbsp;';
    unset($element['#title']);

    return $element;

}


/**
 * Валидация данных формы фильтрации
 * @param array $form - Массив формы
 * @param array $form_state - Текущее состояние формы
 */
function custom_exchange_rates_filter_form_validate($form, &$form_state) {}


/**
 * Выполнение формы фильтрации
 * @param array $form - Массив формы
 * @param array $form_state - Текущее состояние формы
 */
function custom_exchange_rates_filter_form_submit($form, &$form_state) {

    $values = $form_state['values'];
    $query = array();

    if ($values['currency'])
        $query['currency'] = $values['currency'];
    if ($values['date'])
        $query['date'] = custom_exchange_rates_get_timestamp_from_date($values['date']);

    drupal_goto(current_path(), array('query' => $query));

}


/**
 * Возвращает массив валют
 *
 * @return array $currency_array - Массив валют
 * * int currency_id - идентификатор валюты
 * * string currency_name - название валюты
 */
function custom_exchange_rates_get_currency_names() {

    $query = db_select('custom_exchange_rates_currency', 'currency');
    $query->fields('currency', array('currency_id', 'currency_name'));
    $currencies = $query->execute()->fetchAll();

    $currency_array = array();

    foreach ($currencies as $currency)
        $currency_array[$currency->currency_id] = $currency->currency_name;

    return $currency_array;

}


/**
 * Возвращает временную метку по дате формата Y-m-d
 *
 * @param string $date - Дата формата Y-m-d
 * @return int timestamp - Временная метка
 */
function custom_exchange_rates_get_timestamp_from_date($date){

    list($year, $month, $day) = explode('-', $date);
    $time =  mktime(0, 0, 0, $month, $day, $year);

    return $time;

}