# Как был создан проект Zephyr

Рабочая папка: `C:\Users\Asus\Desktop\test_zef`. Ниже описан порядок выполненных работ и приведены команды для повторения; это не дословный журнал терминала.

## 1. Найден и подготовлен Zephyr

На машине была найдена уже скачанная копия Zephyr. Она перенесена в подкаталог `zephyr`; повторно скачивать ОС не потребовалось. Выбрана версия `v4.4.0`. SDK `1.0.1` установлен в `tools/zephyr-sdk-1.0.1` с ARM-компилятором и штатными OpenOCD/GDB.

Для Python-инструментов создано обычное окружение `.venv` на Python 3.12.10. West workspace связан с локальным каталогом `zephyr`; его настройки находятся в `.west/config`. Через `west update` получены CMSIS и STM32 HAL в `modules`.

Этот этап можно повторить для уже имеющегося каталога `zephyr` следующими командами PowerShell из корня проекта:

```powershell
git -C zephyr checkout v4.4.0
py -3.12 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install west==1.5.0
west init -l zephyr
west update cmsis cmsis_6 hal_stm32
python -m pip install @((west packages pip) -split ' ')
west zephyr-export
west sdk install --version 1.0.1 --install-dir tools/zephyr-sdk-1.0.1 --gnu-toolchains arm-zephyr-eabi
```

Это команды первоначальной подготовки: в существующем workspace повторно выполнять `west init` и создавать `.venv` не нужно. CMake и Ninja должны быть доступны в системном `PATH`. При ручной распаковке SDK его стандартная регистрация выполняется командой:

```powershell
cmake -P tools/zephyr-sdk-1.0.1/cmake/zephyr_sdk_export.cmake
```

## 2. Создано приложение

Приложение написано в `application`, без изменения исходников Zephyr. Использован готовый board `blackpill_f401cc`, соответствующий `blackpill_f401cc/stm32f401xc`.

| Созданный файл | Чем наполнен |
| --- | --- |
| `application/CMakeLists.txt` | `find_package(Zephyr)`, три исходника приложения и каталог `include` |
| `application/prj.conf` | `CONFIG_SERIAL=y` и `CONFIG_GPIO=y` |
| `application/app.overlay` | Настройки USART2, GPIO и alias для UART |
| `application/src/main.c` | Инициализация GPIO и запуск UART-обработки |
| `application/src/binary_protocol.c` | UART-код, вынесенный из `main.c` |
| `application/include/binary_protocol.h` | Объявление функции UART-модуля |
| `application/src/gpio_reaction.c` | GPIO-код с callback и отложенной задачей Zephyr |
| `application/include/gpio_reaction.h` | Объявление функции инициализации GPIO |

Сначала добавлены CMake и Kconfig, затем overlay и UART-код. После этого UART-код вынесен в отдельный файл. Позже таким же образом добавлен отдельный GPIO-файл и включён в `target_sources`.

В `scripts/uart_increment_client.py` отдельно создан запрошенный Windows-клиент. В `scripts/requirements.txt` записано `pyserial==3.5`. Этот скрипт не участвует в сборке прошивки.

## 3. Выполнена сборка

В каждой новой сессии PowerShell из корня проекта:

```powershell
.\.venv\Scripts\Activate.ps1
west build -b blackpill_f401cc application -d build
```

Повторная сборка:

```powershell
west build -d build
```

Сборка выполнена штатными `west`, CMake, Ninja и Zephyr SDK. Результат — `build/zephyr/zephyr.elf`; рядом создаются `.bin`, `.hex`, `.config` и `zephyr.dts`. Проверенная сборка занимает 19 616 байт FLASH и 5 824 байта RAM.

## 4. Настроены ST-Link и F5

Создан `openocd/stm32f401cc-stlink.cfg`: ST-Link, SWD, STM32F4 и программный сброс без NRST. Это аппаратный конфиг OpenOCD, не скрипт сборки.

В `.vscode/tasks.json` оставлены `build`, `clean`, `flash` с прямыми вызовами `west`. В `.vscode/launch.json` создана конфигурация `STM32`: перед F5 выполняется задача `build`, затем Cortex-Debug загружает ELF через OpenOCD. В `.vscode/settings.json` оставлены настройки редактора. `extensions.json` удалён.

Для F5 требуется установленный Cortex-Debug и подключённый ST-Link. Вручную загрузка вызывается так:

```powershell
west flash -d build -r openocd -- --config openocd/stm32f401cc-stlink.cfg --verify
```

Сборка проверена; физическая загрузка на этой машине пока не проверена.

## 5. Добавлен Git и убрано лишнее

Инициализирован Git с веткой `main`. Приложение, настройки и документация добавлялись короткими коммитами. На GitHub создан приватный репозиторий. Команды для нового репозитория:

```powershell
git init -b main
git remote add origin https://github.com/Pashkorb/zephyr-stm32f401ccu6-uart.git
git push -u origin main
```

Между инициализацией и первым push файлы добавлялись через `git add` и сохранялись через `git commit`. В существующем репозитории Git и remote уже настроены. Реальный порядок изменений можно посмотреть командой `git log --reverse --oneline`.

Зависимости `zephyr`, `modules`, SDK, `.venv`, `.west` и результаты `build` исключены через `.gitignore`. Добавленные мной тесты и временные проверочные сборки удалены. Linux-модуль, его клиент, конфигурация ядра и собранный `.ko` вынесены из этого проекта в отдельную папку `C:\Users\Asus\Desktop\nanopi-fire3-uart`.

Перенос соответствует следующим командам из корня проекта; он уже выполнен, повторять их не нужно:

```powershell
Move-Item linux_module ..\nanopi-fire3-uart
Move-Item build/linux-fire3 ..\nanopi-fire3-uart/build
Move-Item tools/nanopi-fire3-image.config ..\nanopi-fire3-uart/kernel.config
```
