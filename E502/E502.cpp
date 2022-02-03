#include "l502api.h"
#include "e502api.h"
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
#include <iostream>
#include <thread> // для работы с потоками
#include <fstream>
#include <iomanip> // округление
#include <vector>

#define M_PI 3.14159265358979323846
#define DIGITS_AFTER_DECIMAL_POINT 4
#pragma comment (lib, "x502api.lib") // подключение статических библиотек, 
#pragma comment (lib, "e502api.lib") // находящихся в папке проекта

using namespace std;

bool print_quantity_connect(int value) /* проверка наличия подключения модулей и
                                       вывод количества подключенных модулей */
{
    bool connect = false; // наличие подключенных модулей
    if (value < 0)
        cout << "Ошибка получения списка серийных номеров" << endl;
    else if (value == 0)
        cout << "Не найдено ни одного модуля" << endl;
    else
    {
        connect = true;
        if (value == 1)
            cout << "Найден ";
        else
            cout << "Найдено ";
        cout << value << " модул";
        if ((value % 10) == 1)
            cout << "ь" << endl;
        else if ((value % 10) > 1 && (value % 10) < 5)
            cout << "я" << endl;
        else
            cout << "ей" << endl;
    }
    return connect;
}

void print_info_connected_modules(int quantity, t_x502_devrec* devrec_list)
/* Вывод информации об имеющихся модулях */
{
    for (int i = 0; i < quantity; i++)
    {
        cout << endl << "Модуль " << i << endl << "Наименование: "
            << devrec_list[i].devname << endl << "Серийный номер: "
            << devrec_list[i].serial << endl << "Тип подключения: ";
        devrec_list[i].iface == X502_IFACE_PCI ? cout << "PCI/PCIe" : cout << "";
        devrec_list[i].iface == X502_IFACE_USB ? cout << "USB" : cout << "";
        devrec_list[i].iface == X502_IFACE_ETH ? cout << "Ethernet" : cout << "";
        cout << endl;
    }
}

void open_connection_module(int32_t& err, uint32_t& tout, t_x502_hnd& hnd, uint32_t& mode, bool& check)
// установление режима работы модуля по USB
{
    uint32_t usb_devcnt = 0;
    int32_t fnd_devcnt = 0; /* общее кол-во найденных записей */
    t_x502_devrec* devrec_list = (t_x502_devrec*)malloc(1 * sizeof(t_x502_devrec)); /* список записей о устройствах */
    /* Выбор платы и установка параметров работы */
    E502_UsbGetDevRecordsList(devrec_list, 0, 0, &usb_devcnt); /* Получить список записей, соответствующих подключенным модулям E502 */
    bool connect = print_quantity_connect(usb_devcnt); /* поскольку работаем только с е502 по USB */
    if (connect)
        cout << "Количество подключенных по USB модулей: " << usb_devcnt << endl;
    if ((usb_devcnt) != 0)
    {
        devrec_list = (t_x502_devrec*)malloc(usb_devcnt * sizeof(t_x502_devrec));
        if (devrec_list != NULL)
        {
            int32_t res = E502_UsbGetDevRecordsList(&devrec_list[fnd_devcnt], usb_devcnt, 0, NULL); // получаем записи о модулях E-502, подключенных по USB
            if (res >= 0)
            {
                check = true;
                fnd_devcnt += res;
            }
        }
        cout << "Список устройств доступных для выбора: " << endl;
        print_info_connected_modules(fnd_devcnt, devrec_list);
    }
    if (fnd_devcnt != 0)
    {
        cout << endl << "Укажите номер модуля для дальнейшей работы с ним (от 0 до "
            << fnd_devcnt - 1 << ")" << endl;
        uint32_t select_modul;
        while (true)
        {
            cin >> select_modul;
            if (select_modul < 0 || select_modul >(fnd_devcnt - 1))
                cout << "Некорректно введен номер модуля. Повторите ввод" << endl;
            else
                break;
        }
        cout << "Выбран " << select_modul << " модуль" << endl;
        hnd = X502_Create();
        if (hnd == NULL)
            cout << "Ошибка создания описателя модуля" << endl;
        else
        {
            err = X502_OpenByDevRecord(hnd, &devrec_list[select_modul]);
            if (err != X502_ERR_OK)
                cout << "Возникла ошибка установления соединения" << endl;
            else
            {
                err = X502_SetMode(hnd, mode); // установка режима работы модуля (0 - ПЛИС, без BlackFin; 1 - через BlackFin)
                if (err == X502_ERR_OK)
                {
                    if (mode == 0)
                        cout << "Все потоки данных передаются через ПЛИС, минуя сигнальный процессор BlackFin" << endl;
                    else if (mode == 1)
                        cout << "Все потоки данных передаются через сигнальный процессор, который должен быть загружен прошивкой для обработки этих потоков" << endl;
                    else if (mode == 2)
                        cout << "Отладочный режим" << endl;
                }
            }
        }
        X502_FreeDevRecordList(devrec_list, fnd_devcnt); // освобождение ресурсов действительных записей из списка
        free(devrec_list);
    }
}

void set_parametr(t_x502_hnd hnd, int32_t err, uint32_t Fd)
{
    uint32_t value_divider = X502_REF_FREQ_2000KHZ / Fd; // опорная частота/частота дискретизации
    t_x502_sync_mode SyncMode = X502_SYNC_INTERNAL; // Выбор источника опорной частоты //= X502_SYNC_INTERNAL = 0, //= < Внутренний сигнал
    err = X502_SetRefFreq(hnd, X502_REF_FREQ_2000KHZ); //= Установление значения опорной частоты синхронизации
    if (err != X502_ERR_OK)
        printf("X502_SetIntRefFreq() Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Установленная внутренняя опорная частота синхронизации = " <<
        X502_REF_FREQ_2000KHZ << " Гц" << endl;
    err = X502_SetSyncMode(hnd, SyncMode); //= Установка режима генерации частоты синхронизации
    if (err != X502_ERR_OK)
        printf("X502_SetSyncMode() Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Источник опорной частоты: " << SyncMode << endl;

    uint32_t lch_cnt = 1; // количество логических каналов (используемых по факту)
    err = X502_SetLChannelCount(hnd, lch_cnt); /* установка количества логических каналов */
    if (err != X502_ERR_OK)
        printf("X502_SetLChannelCount Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Установленное количество логических каналов: " << lch_cnt << endl;
    /* Установка параметров логического канала: измерение напряжения (+- 1В) относительно общей земли */
    /* Описатель модуля, лог. канал, физ. канал, режим, амплитуда напр., коэф. усред. */
    err = X502_SetLChannel(hnd, 0, 0, X502_LCH_MODE_COMM, X502_ADC_RANGE_5, 0);
    if (err != X502_ERR_OK)
        printf("X502_SetLChannel Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Были установлены следующие параметры " << 0 << "-го лог. канала:" << endl
        << "Режим измерения канала АЦП: " << X502_LCH_MODE_COMM << endl <<
        "Диапазон измерения для канала АЦП: " << X502_ADC_RANGE_5 << endl <<
        "Коэффициент усредненения по каналу: " << 0 << endl;

    double f_frame = 0;
    double f_acq = Fd;
    err = X502_SetAdcFreq(hnd, &f_acq, &f_frame); //= Установка частоты сбора АЦП
    if (err != X502_ERR_OK)
        printf("X502_SetAdcFreq Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Значение межкадровой задержки = " << f_frame << endl
        << "Значение частоты сбора АЦП = " << f_acq << endl;

    /* установка делителя частоты синхронного вывода */
    err = X502_SetOutFreqDivider(hnd, value_divider);
    if (err != X502_ERR_OK)
        printf("X502_SetOutFreqDivider Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Установлен делитель частоты синхронного вывода = " << value_divider << "\t"
        << X502_REF_FREQ_2000KHZ << "/" << Fd << endl;
    err = X502_StreamsEnable(hnd, X502_STREAM_DAC1 | X502_STREAM_DAC2 | X502_STREAM_ADC);
    if (err != X502_ERR_OK)
        printf("X502_StreamsEnable Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Получено разрешение синхронных потоков на вывод и ввод" << endl;
    /* передача установленных настроек в модуль */
    err = X502_Configure(hnd, 0);
    if (err != X502_ERR_OK)
        printf("X502_Configure Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Установленные настройки переданы в модуль" << endl;
}

void create_signal(double* signal, uint32_t sample_buf, double Fn, uint32_t Fd, double& phase, double A)
{
    double delta_fi = 2.0 * M_PI * Fn / Fd; // приращение фазы
    for (uint32_t i = 0; i < sample_buf; i++)
    {
        signal[i] = A * sin(phase);
        phase += delta_fi;
        if (phase > M_PI)
            phase -= 2 * M_PI; // чтобы фаза была в пределах [-pi; pi]
    }
}

void preparation_and_transmission_data(t_x502_hnd& hnd, int32_t& err, double* signal_CH1, uint32_t& size_out_buf, double& Fn, uint32_t& Fd, double& phase_CH1,
    uint32_t* out_buf, uint32_t& tout, double& A)
    // функции для подготовки данных и последующей передачи (страница руководства 24)
{
    create_signal(signal_CH1, size_out_buf, Fn, Fd, phase_CH1, A);
    err = X502_PrepareData(hnd, signal_CH1, NULL, NULL, size_out_buf, X502_DAC_FLAGS_VOLT, out_buf);
    if (err != X502_ERR_OK)
        printf("X502_PrepareData Error = %d: %s!\n", err, X502_GetErrorString(err));
    err = X502_Send(hnd, out_buf, size_out_buf, tout);
    if (err < 0)
        printf("X502_Send Error = %d: %s!\n", err, X502_GetErrorString(err));
}

void streaming_output_input(t_x502_hnd& hnd, int32_t& err, uint32_t size, double Fn, uint32_t Fd, double A, uint32_t tout, uint32_t step)
{
    uint32_t* out_buf = new uint32_t[size]; /* Выходной массив, в который будут сохранены сформированные отсчеты */
    for (uint32_t i = 0; i < size; i++)
    {
        out_buf[i] = 0.0;
    }
    uint32_t size_in_buf = size;
    uint32_t size_out_buf = size_in_buf;
    uint32_t* input_buf = new uint32_t[size_in_buf];
    double* input_data = new double[size_in_buf]; /* Массив, в который будут сохранены преобразованные данные от АЦП */
    ofstream input_CH1;
    input_CH1.open("input_CH1.txt");
    input_CH1 << fixed << showpoint << setprecision(DIGITS_AFTER_DECIMAL_POINT);
    double* signal_CH1 = new double[size];
    static double phase_CH1 = 0;
    err = X502_PreloadStart(hnd);
    if (err != X502_ERR_OK)
        printf("X502_PreloadStart Error = %d: %s!\n", err, X502_GetErrorString(err));
    preparation_and_transmission_data(hnd, err, signal_CH1, size_out_buf, Fn, Fd, phase_CH1, out_buf, tout, A);
    err = X502_StreamsStart(hnd);
    if (err != X502_ERR_OK)
        printf("X502_StreamsStart Error = %d: %s!\n", err, X502_GetErrorString(err));
    for (size_t i = 0; i < step; i++)
    {
        /************************ СИНХРОННЫЙ ПОТОКОВЫЙ ВЫВОД ************************/
        preparation_and_transmission_data(hnd, err, signal_CH1, size_out_buf, Fn, Fd, phase_CH1, out_buf, tout, A);
        /***************************************************************************/
        /************************ СИНХРОННЫЙ ПОТОКОВЫЙ ВВОД ************************/
        err = X502_Recv(hnd, input_buf, size_in_buf, tout);
        if (err < 0)
            printf("X502_Recv Error = %d: %s!\n", err, X502_GetErrorString(err));
        err = X502_ProcessAdcData(hnd, input_buf, input_data, &size_in_buf, X502_PROC_FLAGS_VOLT);
        if (err != X502_ERR_OK)
            printf("X502_ProcessAdcData Error = %d: %s!\n", err, X502_GetErrorString(err));
        for (size_t i = 0; i < size_in_buf; i++)
        {
            input_CH1 << input_data[i] << endl;
        }
        /***************************************************************************/
    }
    input_CH1.close();
    delete[] out_buf;
    delete[] signal_CH1;
    delete[] input_buf;
    delete[] input_data;
    err = X502_StreamsStop(hnd); // Останов синхронных потоков ввода/вывода
    if (err != X502_ERR_OK)
        printf("X502_StreamsStop Error = %d: %s!\n", err, X502_GetErrorString(err));
}

void close_connection_module(int32_t& err, t_x502_hnd& hnd) // закрытие соединения с модулем
{
    err = X502_Close(hnd);
    if (err != X502_ERR_OK)
        printf("X502_Close Error = %d: %s!\n", err, X502_GetErrorString(err));
    else
        cout << "Соединение с модулем закрыто" << endl;
    X502_Free(hnd);
    hnd = NULL;
}

int main()
{
    setlocale(LC_ALL, "rus");
    t_x502_hnd hnd = NULL; /* описатель открытого устройства */
    int32_t err = X502_ERR_OK;
    uint32_t tout = 1000; // таймаут (в мс) на время ожидания события
    uint32_t mode = X502_MODE_FPGA; // режим работы модуля
    double Fn = 1800; // несущая частота, Гц
    double A = 1.0; // амплитуда колебания
    uint32_t Fd = 10000; // частота дискретизации, Гц (должна быть кратна опроной (2 МГц))
    if (X502_REF_FREQ_2000KHZ % Fd)
    {
        cout << "Опорная частота не кратна частоте дискретизации." << endl << "Исправьте значение переменной Fd" << endl;
        return 0;
    }
    bool check_modules = false;
    open_connection_module(err, tout, hnd, mode, check_modules);
    if (!check_modules)
        return 0;
    set_parametr(hnd, err, Fd);
    uint32_t size_buf = 1000; // количество отсчетов в одном буфере
    uint32_t step = 200; // количество проходок цикла
    t_x502_hnd hnd_in = hnd;
    //thread output(streaming_output, hnd, err, size_buf, Fn, Fd, A, tout, step*2);
    //output.detach();
    streaming_output_input(hnd, err, size_buf, Fn, Fd, A, tout, step);
    //thread input(streaming_input, hnd_in, err, size_buf, Fn, Fd, A, tout, step);
    //input.join();
    close_connection_module(err, hnd);
    return 0;
}