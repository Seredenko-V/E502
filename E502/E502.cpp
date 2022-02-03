// e-502.cpp : Defines the entry point for the console application.
//

#include "e502api.h"
#include <iostream>
#include <clocale>
#include <cstdlib>
#include <cmath> 
#include <fstream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <string>
#include <vector>

#pragma comment(lib, "x502api.lib")
#pragma comment(lib, "e502api.lib")


const double PI = 3.141592653589793;

using namespace std;

uint32_t get_devrec_E502(t_x502_devrec** pdevrec_list) // Количество подключенных модулей
{
	int32_t fnd_devcnt = 0; // Общее количество модулей
	uint32_t usb_devcnt = 0; // Количество модулей по USB
	t_x502_devrec* devrec_list = NULL; // Список записей о устройствах 
	E502_UsbGetDevRecordsList(NULL, 0, 0, &usb_devcnt); // Получить список записей, соответствующих подключенным модулям E502
	if (usb_devcnt > 0) // Если подключен хотя бы один модуль по USB
	{
		//devrec_list = (t_x502_devrec*)malloc(usb_devcnt * sizeof(t_x502_devrec)); // Выделение памяти для сохранения найденного количества записей о устройствах 
		devrec_list = new t_x502_devrec[usb_devcnt];
		int32_t res = E502_UsbGetDevRecordsList(&devrec_list[fnd_devcnt], usb_devcnt, 0, NULL);
		if (res > 0)
		{
			fnd_devcnt += res;

		}
	}
	if (fnd_devcnt != 0) // Если подключен хотя бы один модуль, то сохраняем указатель на выделенный массив
	{
		*pdevrec_list = devrec_list;
	}
	else
	{
		//free(devrec_list);
		delete[] devrec_list;
	}
	return fnd_devcnt;
}


void select_dev_open(t_x502_hnd& hnd, t_x502_devrec* devrec_list, const int32_t& fnd_devcnt) // Выбор модуля
{
	//t_x502_hnd hnd = NULL; // Описатель модуля
	uint32_t dev_ind;
	uint32_t ver_lib = X502_GetLibraryVersion(); // Получить версию библиотеки
	cout << "Модулей доступно:" << fnd_devcnt << endl;
	for (int i = 0; i < fnd_devcnt; i++)
	{
		cout << "Устройство: " << devrec_list[i].devname << endl;
		cout << "Серийный номер: " << devrec_list[i].serial << endl;
		cout << endl;

	}
	cout << "Введите номер модуля, с которым хотите работать (от 0 до " << fnd_devcnt - 1 << "):";
	while (true)
	{
		cin >> dev_ind;
		if (dev_ind >= fnd_devcnt || dev_ind < 0)
		{
			cout << "Неверно указан номер модуля. Повторите ввод: ";
		}
		else
		{
			cout << endl;
			cout << "Выбран модуль: " << dev_ind << endl;
			hnd = X502_Create(); // Создание описателя модуля
			cout << "Указатель: " << hnd << endl;
			break;
		}
	}
	if (hnd != NULL)
	{
		X502_OpenByDevRecord(hnd, &devrec_list[dev_ind]); // Открыть соединение с модулем по записи о устройстве 
		t_x502_info info; // Информация о модулях
		int32_t err; // Ошибка
		err = X502_GetDevInfo(hnd, &info);
		if (err != X502_ERR_OK)
		{
			cout << stderr << "- Ошибка получения серийной информации о модуле: " << X502_GetErrorString(err);
		}
		else
		{
			cout << "Установлена связь со следующим модулем:" << endl;
			cout << "  Серийный номер          : " << info.serial << endl;
			cout << "  Наличие ЦАП             : ";
			info.devflags& X502_DEVFLAGS_DAC_PRESENT ? cout << "Да" : cout << "Нет";
			cout << endl;
			cout << "  Наличие BlackFin        : ";
			info.devflags& X502_DEVFLAGS_BF_PRESENT ? cout << "Да" : cout << "Нет";
			cout << endl;
			cout << "  Наличие гальваноразвязки: ";
			info.devflags& X502_DEVFLAGS_GAL_PRESENT ? cout << "Да" : cout << "Нет";
			cout << endl;
			cout << "  Индустриальное исп.     : ";
			info.devflags& X502_DEVFLAGS_INDUSTRIAL ? cout << "Да" : cout << "Нет";
			cout << endl;
			cout << "  Наличие интерф. PCI/PCIe: ";
			info.devflags& X502_DEVFLAGS_IFACE_SUPPORT_PCI ? cout << "Да" : cout << "Нет";
			cout << endl;
			cout << "  Наличие интерф. USB     : ";
			info.devflags& X502_DEVFLAGS_IFACE_SUPPORT_USB ? cout << "Да" : cout << "Нет";
			cout << endl;
			cout << "  Наличие интерф. Ethernet: ";
			info.devflags& X502_DEVFLAGS_IFACE_SUPPORT_ETH ? cout << "Да" : cout << "Нет";
			cout << endl;
			cout << "  Версия ПЛИС             : " << ((info.fpga_ver >> 8) & 0xFF, info.fpga_ver & 0xFF);
			cout << endl;
			cout << "  Версия PLDA             : " << info.plda_ver << endl;
			cout << endl;
		}
	}
}


void setup_params_module(const t_x502_hnd& hnd, const uint32_t& Fd) // Установка параметров модуля
{
	cout << "Установка параметров модуля:" << endl;
	int32_t err; // Код ошибки
	/*----------Установка количества логических каналов----------*/
	uint32_t lch_cnt = 1; // Количество логических каналов
	err = X502_SetLChannelCount(hnd, lch_cnt); // Установка количества логических каналов
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки количества логических каналов: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлено логических каналов: " << lch_cnt << endl;
	}
	/*----------Установка параметров логического канала №1----------*/
	uint32_t lch = 0; // Номер логического канала от 0 до 255
	uint32_t phy_ch = 0; // Номер физического канала АЦП начиная с 0 (0-15 для дифференциального режима, 0-31 для режима с общей землей)
	uint32_t mode_lch = X502_LCH_MODE_COMM; // Режим измерения канал АЦП (0 - Измерение напряжения относительно общей земли; 1 - Дифференциальное измерение напряжения; 2 - Измерение собственного нуля
	uint32_t range = X502_ADC_RANGE_5; // Диапазон измерения канала (0 - Диапазон +/-10V; 1 - Диапазон +/-5V; 2 - Диапазон +/-2V; 3 - Диапазон +/-1V; 4 - Диапазон +/-0.5V; 5 - Диапазон +/-0.2V;
	uint32_t avg = 0; // Коэффициент усреднения по каналу (Нулевое значение соответствует значению коэффициента, определенного библиотекой)
	err = X502_SetLChannel(hnd, lch, phy_ch, mode_lch, range, avg); // Установка параметров логического канала
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки параметров логического канала №1: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлены параметры логического канала №1" << endl;
	}
	/*----------Установка значения внутренней опорной частоты синхронизации----------*/
	uint32_t freq = X502_REF_FREQ_2000KHZ;
	err = X502_SetRefFreq(hnd, freq); // Установка значения внутренней опорной частоты синхронизации
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки значения внутренней опорной частоты синхронизации: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлена опорная частота:" << X502_REF_FREQ_2000KHZ << endl;
	}
	/*----------Установка делителя частоты сбора для АЦП----------*/
	uint32_t adc_freq_div = freq / Fd / lch_cnt; // Значение делителя частоты АЦП от 1 до 1024
	err = X502_SetAdcFreqDivider(hnd, adc_freq_div); // Установка делителя частоты сбора для АЦП
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки делителя частоты сбора для АЦП: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлен делитель частоты сбора для АЦП: " << adc_freq_div << endl;
	}

	/*----------Установка значения межкадровой задержки для АЦП----------*/
	uint32_t delay = 0; // Значение межкадровой задержки от 0 до 0x1FFFFF
	err = X502_SetAdcInterframeDelay(hnd, delay); // Установка значения межкадровой задержки для АЦП
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки межкадровой задержки для АЦП: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлена межкадровая задержка для АЦП: " << delay << endl;
	}
	/*----------Установка делителя частоты синхронного вывода----------*/
	uint32_t out_freq_div = freq / Fd; // Делитель частоты синхронного вывода от 2 до 1024
	err = X502_SetOutFreqDivider(hnd, out_freq_div); // Установка делителя частоты синхронного вывода
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки делителя синхронного вывода: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлен делитель частоты синхронного вывода: " << out_freq_div << endl;
	}
	/*----------Установка режима генерации частоты синхронизации----------*/
	uint32_t sync_mode = X502_SYNC_INTERNAL; // Значение из t_x502_sync_mode, определяющее кто будет источником частоты синхронизации (0 - Внутренний сигнал; 1 - От внешнего мастера по разъему межмодульной синхронизации.
	err = X502_SetSyncMode(hnd, sync_mode); // Установка режима генерации частоты синхронизации
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки режима генерации частоты синхронизации: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлен режим генерации частоты синхронизации: " << sync_mode << endl;
	}
	/*----------Установка режим работы модуля----------*/
	uint32_t mode = X502_MODE_FPGA; // Режим работы модуля (0 - Все потоки данных передаются через ПЛИС минуя сигнальный процессор BlackFin; 1 - Все потоки данных передаются через сигнальный процессор, который должен быть загружен прошивкой для обработки этих потоков; 3 - Отладочный режим)
	err = X502_SetMode(hnd, mode); // Установка режим работы модуля
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки режима работы модуля: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлен режим работы модуля: " << mode << endl;
	}

	/*----------Установка делителя частоты синхронного ввода с цифровых линий----------*/
	uint32_t din_freq_div = freq / Fd; // Делитель частоты синхронного ввода с цифровых линий
	err = X502_SetDinFreqDivider(hnd, din_freq_div); // Установка делителя частоты синхронного ввода с цифровых линий
	if (err != X502_ERR_OK)
	{
		cout << stderr << "- Ошибка установки делителя синхронного ввода с цифровых линий: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Установлен делитель частоты синхронного ввода с цифровых линий: " << din_freq_div << endl;
	}
	/*----------Передача установленных настроек в модуль----------*/
	uint32_t flags = 0; // Флаги (резерв - должно быть равно 0)
	err = X502_Configure(hnd, flags); // Передача установленных настроек в модуль
	if (err != X502_ERR_OK)
	{
		cout << stderr << "Ошибка передачи установленных настроек в модуль: " << X502_GetErrorString(err) << endl;
	}
	else
	{
		cout << "Передача установленных настроек в модуль" << endl;
	}
	cout << "Параметры модуля установлены" << endl;
}


void generate_saw(double* signal, const double& Fn, const uint32_t& Fd, const int& sample, double& phi)
{
	double ts = 1.0 / Fd; // Время между отсчётами
	uint16_t length = 100; // Количество слагаемых в ряде Фурье
	uint16_t  A = 1; // Амплитуда
	double delte_phi = 2.0 * 3.14 * ts * Fn;
	double tmp;
	for (size_t i = 0; i < sample; i++)
	{
		tmp = 0;
		for (size_t j = 1; j <= length; j++)
		{
			tmp += pow(-1, j + 1) * sin(phi * j) / j;
		}
		phi += delte_phi;
		if (phi > 3.14)
		{
			phi -= 2 * 3.14;
		}
		signal[i] = A * 2.0 / 3.14 * tmp;
	}
}


void generate_saw_bad(double* signal, double Fn, uint32_t Fd, int sample, double& phi)
{
	double A = 2.0 / sample;
	for (size_t i = 0; i < sample; i++)
	{
		signal[i] = A * i;
	}
}


void generate_meandr(double* signal, double Fn, uint32_t Fd, int sample, double& phi) // ИСПРАВИТЬ
{
	double ts = 1.0 / Fd; // Время между отсчётами
	uint16_t  length = 20; // Количество слагаемых в ряде Фурье
	double delte_phi = 2.0 * 3.14 * ts * Fn;
	uint16_t A = 1; // Амплитуда
	double tmp;
	for (size_t i = 0; i < sample; i++)
	{
		tmp = 0;
		for (size_t j = 1; j <= length; j += 2)
		{
			tmp += pow(-1, j + 1) * sin(phi * j) / j;
		}
		phi += delte_phi;
		if (phi > 3.14)
		{
			phi -= 2 * 3.14;
		}
		signal[i] = A * 4.0 / 3.14 * tmp;
	}
}


void generate_sin(double* signal, const double& Fn, const uint32_t& Fd, const uint32_t& sample, double& phi)
{
	double ts = 1.0 / Fd; // Время между отсчётами
	double delte_phi = 2 * PI * ts * Fn;
	uint16_t A = 1; //амплитуда
	for (int32_t i = 0; i < sample; i++)
	{
		signal[i] = 1 * sin(phi);
		phi += delte_phi;
		if (phi > PI)
		{
			phi -= 2 * PI;
		}
	}
}


uint32_t my_mod(int32_t number, uint32_t mod)
{
	if (number >= 0)
	{
		return number % mod;
	}
	while (number < 0)
	{
		number += mod;
	}
	return number % mod;
}


uint32_t inverse_number(uint32_t number, uint32_t mod)
{
	return uint32_t(pow(number, mod - 2)) % mod;
}


void generation_psp(uint32_t* psp, uint32_t psp_size, uint32_t* polynomial, uint32_t polynomial_size, uint32_t mod)
{
	psp[0] = 1;
	if (polynomial[polynomial_size - 1] != 1)
	{
		uint32_t inverse;
		inverse = inverse_number(polynomial_size - 1, mod);
		for (size_t i = 0; i < polynomial_size; i++)
		{
			polynomial[i] = my_mod(polynomial[i] * -1 * inverse, mod);
		}
	}
	else
	{
		for (size_t i = 0; i < polynomial_size; i++)
		{
			polynomial[i] = my_mod(polynomial[i] * -1, mod);
		}
	}
	uint32_t sm = 0;
	for (int32_t k = 1; k < psp_size; k++)
	{
		sm = 0;
		for (int32_t i = polynomial_size - 1; i > 0; i--)
		{
			if (polynomial[i] != 0 && k - i >= 0)
			{
				sm += psp[k - i] * polynomial[i];
			}
		}
		psp[k] = my_mod(sm, mod);
	}
}


void modulation(double* signal, double Fn, uint32_t Fd, uint32_t size, double& phi, uint32_t* bits, uint32_t bits_size)
{
	double ts = 1.0 / Fd; // Время между отсчётами
	int sample_int_period = int(Fd) / Fn;
	double delte_phi = 2 * PI * ts * Fn;
	uint16_t A = 1; //амплитуда
	for (size_t j = 0; j < bits_size; j++)
	{
		for (size_t i = 0; i < sample_int_period; i++)
		{
			signal[i + j * sample_int_period] = A * sin(phi);
			phi += delte_phi;
			if (phi > 3.14)
			{
				phi -= PI;
			}
		}
	}
}


void filling_inf_sequence_bits(uint32_t* bits, uint32_t bits_size)
{
	for (size_t i = 0; i < bits_size; i++)
	{
		bits[i] = 0;
	}
}


void generate_meandr_for_DIN(uint32_t* signal, uint32_t sample)
{
	for (size_t i = 0; i < sample; i++)
	{
		if (i % 2 == 0)
		{
			signal[i] = 0x00002; // 0x20001
		}
		else
		{
			signal[i] = 0x00001;
		}

	}
}


uint32_t autocorrelation_function(double* signal, uint32_t signal_size, double* psp, uint32_t psp_size)
{
	double acf_max = 0;
	double acf;
	uint32_t acf_max_pos = 0;
	for (int32_t i = 0; i < signal_size - psp_size + 1; i++)
	{
		acf = 0;
		for (int32_t k = 0; k < psp_size; k++)
		{
			acf += signal[k + i] * psp[k];
		}
		acf /= psp_size;
		if (acf > acf_max)
		{
			acf_max = acf;
			acf_max_pos = i;
		}
	}
	return acf_max_pos;
}


void creat_psp_signal(uint32_t* psp, uint32_t psp_size, uint32_t* psp_signal)
{
	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < psp_size; j++)
		{
			psp_signal[j + i * psp_size] = psp[j];
		}
	}
}


void synchron_mode_setup(const t_x502_hnd& hnd, const double& Fn, const uint32_t& Fd, double* signal_out_CH1, const uint32_t& size, double& phi_CH, const uint32_t& size_out_buf, uint32_t* out_buf, const int32_t& flags, const uint32_t& tout) // Синхронный режим работы
{
	cout << "\nСинхронный режим работы:" << endl;
	int32_t err; // Код ошибки
	/*----------Разрешение синхронных потоков на ввод/вывод----------*/
	uint32_t streams = X502_STREAM_DAC1 | X502_STREAM_ADC; // — Набор флагов t_x502_streams, указывающих, какие потоки должны быть разрешены
	err = X502_StreamsEnable(hnd, streams); // Разрешение синхронных потоков на ввод/вывод
	if (err != X502_ERR_OK)
	{
		std::cout << stderr << "- Ошибка разрешения синхронных потоков на ввод/вывод: " << X502_GetErrorString(err) << std::endl;
	}
	else
	{
		cout << "Разрешение синхронных потоков на ввод/вывод" << std::endl;
	}
	/*----------Начало подготовки вывода синхронных данных----------*/
	err = X502_PreloadStart(hnd); // Начало подготовки вывода синхронных данных
	if (err != X502_ERR_OK)
	{
		std::cout << stderr << "- Ошибка начала подготовки вывода синхронных данных: " << X502_GetErrorString(err) << std::endl;
	}
	else
	{
		std::cout << "Начало подготовки вывода синхронных данных" << std::endl;
	}
	generate_sin(signal_out_CH1, Fn, Fd, size, phi_CH);
	err = X502_PrepareData(hnd, signal_out_CH1, NULL, NULL, size, flags, out_buf); // Подготовка данных для вывода в модуль
	err = X502_Send(hnd, out_buf, size_out_buf, tout); // Передача потоковых данных ЦАП и цифровых выходов в модуль
	/*----------Запуск синхронных потоков ввода/вывода----------*/
	err = X502_StreamsStart(hnd); // Запуск синхронных потоков ввода/вывода
	if (err != X502_ERR_OK)
	{
		std::cout << stderr << "- Ошибка запуска синхронных потоков ввода/вывода: " << X502_GetErrorString(err) << std::endl;
	}
	else
	{
		std::cout << "Запуска синхронных потоков ввода/вывода" << std::endl;
	}
}



void synchron_mode_working(const t_x502_hnd& hnd, const uint32_t& Fd, const double& Fn, const uint32_t& number_of_parcles)
{
	int32_t err; // Код ошибки
	ofstream output("output.txt"); // Создание файла для записи приинимаемой информации с канала 1
	ofstream input("input.txt"); // Создание файла для записи приинимаемой информации с канала 1
	int32_t flags = X502_DAC_FLAGS_VOLT; // Флаги, управляющие работой функции, из t_x502_dacout_flags
	uint32_t tout = 2000; // Таймаут на передачу (в буфер драйвера) данных в мc
	uint32_t size = 10000; // Количество отсчётов в канале на одну операцию обмена с платой, т.е. N=T*Fd
	uint32_t size_out_buf = size; // Количество отсётов в буффере для вывода данных
	uint32_t size_in_buf = size; // Количество отсётов в буффере для ввода данных
	uint32_t* out_buf = new uint32_t[size_out_buf]; // Выходной массив, в который будут сохранены сформированные отсчеты
	uint32_t* in_buf = new uint32_t[size_in_buf]; // Входной массив, в который будут сохранены сформированные отсчеты
	double* signal_out_CH1 = new double[size]; // Выходной сигнал для ЦАП 1
	double* signal_in_CH = new double[size]; // Входной сигнал с АЦП
	double phi_CH = 0; // Фаза сигнала для канала 1
	synchron_mode_setup(hnd, Fn, Fd, signal_out_CH1, size, phi_CH, size_out_buf, out_buf, flags, tout); // Настройка синхронного режима
	for (size_t j = 0; j < number_of_parcles; j++) {
		generate_sin(signal_out_CH1, Fn, Fd, size, phi_CH);
		err = X502_PrepareData(hnd, signal_out_CH1, NULL, NULL, size, flags, out_buf); // Подготовка данных для вывода в модуль
		err = X502_Send(hnd, out_buf, size_out_buf, tout); // Передача потоковых данных ЦАП и цифровых выходов в модуль
		err = X502_Recv(hnd, in_buf, size_in_buf, tout); // Чтение данных АЦП и цифровых входов из модуля
		err = X502_ProcessData(hnd, in_buf, size_in_buf, flags, signal_in_CH, &size, NULL, NULL); // Обработка принятых от модуля данных
		for (size_t i = 0; i < size; i++)
		{
			//output << fixed << showpoint << setprecision(3) << signal_out_CH1[i] << std::endl;
			input << fixed << showpoint << setprecision(3) << signal_in_CH[i] << std::endl;
		}
	}
	output.close(); // Закрытие файла
	input.close(); // Закрытие файла 
	delete[] signal_out_CH1; // Удаление памяти 
	delete[] out_buf; // Удаление памяти
	delete[] in_buf; // Удаление памяти
	delete[] signal_in_CH; // Удаление памяти
	std::cout << "END" << std::endl;
	err = X502_StreamsStop(hnd); // Остановка синхронных потоков ввода/вывода
	if (err != X502_ERR_OK)
	{
		std::cout << stderr << "- Ошибка остановки синхронных потоков ввода/вывода: " << X502_GetErrorString(err) << std::endl;
	}
	else
	{
		std::cout << "Остановлен синхронный поток ввода/вывода" << std::endl;
	}
	//delete[] inf_bits; // Удаление памяти
}


int main()
{
	setlocale(LC_ALL, "rus");
	srand(time(NULL));
	int32_t err; // Код ошибки
	t_x502_hnd hnd = NULL; // Описатель модуля
	t_x502_devrec* devrec_list = NULL; // Количество устройств
	int32_t fnd_devcnt = get_devrec_E502(&devrec_list);
	double Fn = 1800; // Несущая частота
	uint32_t Fd = 16000; // Частота дискретизации
	uint32_t number_of_parcles = 20;
	if (fnd_devcnt != 0)
	{
		select_dev_open(hnd, devrec_list, fnd_devcnt); // Выбор устройства
		setup_params_module(hnd, Fd); // Настройка параметров модуля
		synchron_mode_working(hnd, Fd, Fn, number_of_parcles); // Создание отдельного потока
		X502_Close(hnd); // Закрытие соединения с модулем
		std::cout << "Соединение с модулем закрыто" << std::endl;
		X502_Free(hnd); // Освобождение описателя модуля
	}
	else
	{
		std::cout << "Не найдено ни одного модуля" << endl;
	}
	return 0;
}
