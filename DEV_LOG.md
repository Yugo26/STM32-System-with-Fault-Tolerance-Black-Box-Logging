#    開發日誌

# 

**專案目標：** 實作一個整合感測器、RTOS 多工處理、與 UART 通訊的智慧邊緣裝置。

**硬體平台：** STM32 Nucleo-F411RE (ARM Cortex-M4)

**開發環境：** STM32CubeIDE

---

## [Day 01] 2026-01-12：環境建置與 Hello World

### 完成項目

1. 確認開發板選型 (NUCLEO-F411RE)，理解 STM32 與 ARM 架構關係。
2. 安裝 STM32CubeIDE，建立第一個專案。
3. **GPIO Output：** 成功點亮板載綠色 LED (LD2)。
4. **UART Transmit：** 成功讓板子透過 USB 傳送 "Hello" 字串給電腦終端機。

### 技術筆記

- **HAL_Delay vs osDelay：** 剛開始用 HAL_Delay (Blocking)，後來知道 RTOS 要用 osDelay (Non-blocking)。
- **Printf 重導向：** 嵌入式系統沒有螢幕，必須透過 UART 將 Log 傳到電腦看。

---

## [Day 02] 2026-01-13：深入中斷處理 (Interrupts)

### 完成項目

1. **GPIO Interrupt (EXTI)：** 實作按鈕控制 LED，捨棄輪詢 (Polling) 改用中斷觸發。
2. **Software Debounce：** 解決機械按鈕彈跳問題。
3. **UART Receive Interrupt：** 實作電腦鍵盤輸入 '1'/'0' 控制 LED。

### 技術筆記

- **Callback 機制：** 透過追蹤原始碼，理解 HAL 庫使用 __weak 關鍵字定義 Callback，讓使用者在 main.c 覆寫。
- **Debounce 演算法：** 使用 HAL_GetTick() 檢查時間差，濾除 200ms 內的重複訊號。
- **UART IT 特性：** HAL_UART_Receive_IT 是 One-shot 的，每次 Callback 結束前必須重新啟動接收。

---

## [Day 03] 2026-01-14：導入 FreeRTOS 系統架構

### 完成項目

1. **FreeRTOS 移植：** 使用 CubeMX 啟用 CMSIS_V2 介面。
2. **多工處理 (Multitasking)：** 建立 DefaultTask (閃燈) 與 UART_Task (傳輸)，驗證 Context Switch。
3. **IPC 通訊 (Queue)：** 解決 ISR 不能做複雜運算的問題。
    - ISR 負責：xQueueSendFromISR (快進快出)。
    - Task 負責：xQueueReceive (處理邏輯)。

### 技術筆記

- **Producer-Consumer 模型：** 中斷是生產者，Task 是消費者。
- **Handle 變數：** 發現 Queue Handle 定義在 freertos.c，在 main.c 使用需加 extern。

---

## [Day 04] 2026-01-16：類比世界 (ADC)

### 完成項目

1. **ADC 設定：** 啟用內部溫度感測器 (Internal Temperature Sensor)。
2. **數值轉換：** 實作 Polling 模式讀取 ADC Raw Data。
3. **物理量換算：** 參考 Datasheet，將 0~4095 數值換算為電壓 (mV) 與攝氏溫度。

### 技術筆記

- **Timeout 重要性：** HAL_ADC_PollForConversion 必須設定 Timeout，防止硬體故障導致系統死鎖 (Deadlock)。
- **字串處理：** 使用 sprintf 格式化字串後再透過 UART 傳送，避免多執行緒下的 printf 衝突。

---

## [Day 05] 2026-01-17：PWM 呼吸燈與計時器原理

### 完成項目

1. **Timer 設定：** 設定 TIM2 Channel 1 為 PWM Generation。
2. **數學計算：**
    - PSC (預分頻) = 83 -> Timer 時脈 1MHz (1us)。
    - ARR (週期) = 999 -> PWM 頻率 1kHz (1ms)。
3. **呼吸燈實作：** 在 Task 中動態調整 CCR (Capture Compare Register) 改變佔空比 (Duty Cycle)。

### 技術筆記

- **PWM 原理：** 透過快速切換開關騙過人眼 (視覺暫留)。
- **CCR vs CNT：** 理解硬體比較器如何運作：當 CNT < CCR 時輸出 High，反之 Low。

---

### Next Steps (下一步計畫)

- **I2C/SPI 通訊：** 準備介接外部溫溼度感測器。
- **系統整合：** 將溫度數值 (ADC/I2C) 與 呼吸燈速度 (PWM) 連動。
- **DMA 優化：** 將 UART 傳輸改為 DMA 模式，降低 CPU 負載。

---

## [Day 06] 2026-01-18：系統整合日 (ADC + Queue + PWM)

### 完成項目

1. **整合實作：** 完成「溫控呼吸警示燈」系統。
    - **Producer:** Temperature_Task (每 100ms 讀取 ADC 並轉換溫度，丟入 Queue)。
    - **Consumer:** LED_Task (根據 Queue 收到的溫度值，動態調整 PWM 呼吸頻率)。
2. **Bug 修復 (Debug)：**
    - **變數溢位問題：** 修正 step 變數從 uint8_t 改為 int16_t，解決亮度無法遞減（負數變成大正數）的問題。
    - **效率優化：** 將 HAL_TIM_PWM_Start 移出 for 迴圈，避免重複初始化。
    - **即時性優化：** 將 xQueueReceive 的等待時間設為 0，確保 LED 呼吸動畫流暢，不會因為等不到資料而卡頓。

### 技術筆記

- **Producer-Consumer 模型：** 成功將「邏輯運算」與「硬體控制」分離。即使 ADC 讀取變慢，也不會影響 LED 的呼吸流暢度。
- **Non-blocking 接收：** 在 UI 或動畫類的 Task 中，讀取 Queue 不應使用 Blocking (如 osWaitForever 或長延時)，應使用 0 延時，有資料就更新，沒資料就維持原狀。
- **Unsigned Trap：** 嵌入式開發需特別注意 uint8_t 等無號數在做減法運算時的溢位風險。

## [Day 07] 2026-01-19：通訊協定實戰 (I2C & SPI)

### 完成項目

1. **I2C Scanner (幽靈掃描器)：**
    - 在沒有外部感測器的情況下，利用 HAL_I2C_IsDeviceReady 掃描 1~127 地址。
    - 驗證了 I2C 的 **Handshake 機制**：確認程式能正確識別 NACK (無人回應) 訊號，證明 I2C Driver 運作正常。
2. **SPI Loopback (回環測試)：**
    - 將 **MOSI (PA7)** 與 **MISO (PA6)** 短路。
    - 使用 HAL_SPI_TransmitReceive 驗證 **全雙工 (Full-Duplex)** 通訊。
    - 成功發送 0x55 並同時接收到 0x55，確認 SPI 硬體收發功能正常。

### 技術筆記

- **I2C vs SPI：** I2C 是基於地址應答的 (ACK/NACK)，速度較慢；SPI 是基於移位暫存器交換的 (Shift Register)，速度快且支援全雙工。
- **Loopback 技巧：** 在硬體周邊尚未就緒時，Loopback 是驗證 MCU 自身通訊功能最有效的方法。

---

## [Day 08] 2026-01-20：掉電保存與救磚驚魂 (Internal Flash)

### 完成項目

1. **Flash Driver 實作：** 撰寫 Save_To_Flash 函式，包含 Unlock -> Erase Sector 7 -> Program -> Lock 的完整流程。
2. **Unbricking (救磚成就解鎖) 🏆：**
    - **問題：** 因程式一開機即執行 Flash 擦除，導致 ST-Link 無法連線 (Timeout / Flash Busy)，板子鎖死。
    - **解決：** 使用 **STM32CubeProgrammer**，配合 **Connect Under Reset** + **Hardware Reset** 模式，成功連線並執行 **Full Chip Erase** 救回板子。
3. **溫度紀錄器 (Temperature Recorder)：**
    - 整合 ADC 與 Flash。
    - **流程：** 開機讀取 Flash 顯示上次溫度 -> 讀取目前 ADC 溫度 -> 寫入 Flash 保存。
    - 驗證了斷電後數據不遺失 (Data Persistence)。

### 技術筆記

- **Flash 寫入風險：** 操作 Flash 時若沒有加入開機延遲 (HAL_Delay(2000)), 容易導致 debugger 無法介入而變磚。
- **Sector 規劃：** STM32F411RE 的程式碼位於 Sector 0~5，數據儲存應使用最後的 **Sector 7 (0x08060000)** 以策安全。
- **救援 S.O.P.：** 當 IDE 無法燒錄時，不要驚慌。手動按住 Reset 鍵配合 CubeProgrammer 的 Under Reset 模式通常能搶回控制權。

---

### 目前技能樹狀態 (Skill Tree Status)

- **[v] GPIO & Interrupts** (EXTI, Debounce)
- **[v] Communication** (UART, I2C Scanner, SPI Loopback)
- **[v] Analog** (ADC, PWM)
- **[v] RTOS Kernel** (Task, Queue, Context Switch)
- **[v] System** (Internal Flash, Bootloader concepts)

---

## [Day 09] 2026-01-21：解放 CPU (DMA 原理與實作)

### 完成項目

1. **UART DMA 傳輸 (TX)：**
    - 將原本 Blocking 的 HAL_UART_Transmit 升級為 HAL_UART_Transmit_DMA。
    - **驗證：** 發送長字串時，CPU 仍能同時控制 LED 閃爍，證明傳輸過程不佔用 CPU 資源。
2. **ADC DMA 循環採集 (Circular Mode)：**
    - 設定 ADC 為 **Continuous Conversion**，並啟用 DMA **Circular Mode**。
    - **效果：** 硬體自動在背景更新 adc_buffer 變數，CPU 無需發送 Start 或 Poll 指令，直接讀取記憶體即可獲得最新數值。
3. **Bug 修復 (Pin Conflict)：**
    - **問題：** LED 無法閃爍。
    - **原因：** PA5 腳位被設定為 TIM2_CH1 (PWM 模式)，導致 HAL_GPIO_TogglePin 被硬體忽略。
    - **解決：** 在 CubeMX 將 PA5 改回 GPIO_Output，歸還控制權。

### 技術筆記

- **DMA (Direct Memory Access) 核心概念：**
    - **搬運工理論：** CPU (老闆) 只需下令給 DMA (搬運工) 來源與目的地，搬運過程完全不消耗 CPU 算力。這對未來的 AI 運算至關重要。
- **ADC Circular Mode：**
    - 若使用 Normal Mode，DMA 搬完一次就會停止。監控類應用 (如溫度) 必須使用 Circular Mode，讓 DMA 搬完自動重置指標，形成無限迴圈。
- **C 語言指標觀念釐清：**
    - HAL_ADC_Start_DMA(..., &adc_buffer, ...)：這裡用 & 是因為 DMA 控制器需要知道記憶體的**地址 (Address)** 才能搬運。
    - val = adc_buffer：CPU 讀取時直接取變數的**值 (Value)**，不需加 *。
- **RTOS 整合：**
    - 在 FreeRTOS Task 中，只需在初始化時呼叫一次 Start_DMA，之後 Task Loop 僅需負責讀值與運算，實現了真正的**非同步 (Asynchronous) 採樣**。

### 專案進度 (Project Status)

目前系統架構已升級為：

- **Input:** ADC (DMA 自動更新)
- **Processing:** FreeRTOS Task (負責算數學 & 邏輯)
- **Output:** UART (DMA 發送) + LED (GPIO)
- **Storage:** Internal Flash (掉電保存)

---

## [Day 11] 2026-01-23：代碼重構與 Shell 互動介面 (Modularization & CLI)

### 完成項目

1. **程式碼模組化 (Refactoring)：**
    - 將 main.c 中的功能拆解為獨立模組：
        - my_flash.c/.h：負責 Internal Flash 的讀寫與保護。
        - my_sensor.c/.h：負責 ADC 數值換算與物理量計算。
        - my_shell.c/.h：負責 UART 通訊與指令解析。
    - 實現 **高內聚低耦合 (High Cohesion, Low Coupling)** 架構，主程式僅負責調度 Task。
2. **CLI (Command Line Interface) 實作：**
    - 設計 UART 接收緩衝區 (rx_buffer) 機制，從單字元接收升級為**字串行接收**。
    - 實作指令解析器：輸入 led on 可控制開發板 LED，並回傳狀態訊息。
3. **Bug 修復 (Pointer Issue)：**
    - **問題：** HAL_UART_Transmit 無法發送資料，Console 空白。
    - **原因：** shell_huart 本身已是指標 (UART_HandleTypeDef *)，呼叫 HAL 函式時錯誤地多加了 & (&shell_huart)，導致傳入錯誤的記憶體地址。
    - **解決：** 移除 &，直接傳遞指標變數。

### 技術筆記

- **字串處理細節：**
    - 接收字串時，偵測到 \r 或 \n (Enter鍵) 代表輸入結束。
    - 務必在 Buffer 末端補上 **\0 (Null Terminator)**，否則 strcmp 或 strlen 會讀到記憶體亂碼導致當機。
- **中斷與旗標 (ISR & Flags)：**
    - **ISR (Callback):** 只負責搬運資料到 Buffer 並設立旗標 (cmd_flag = 1)，保持快進快出。
    - **Task (Process):** 負責耗時的字串比對 (strcmp) 與邏輯處理。
- **初始化順序 (Initialization Order)：**
    - 模組的初始化函式 (如 Shell_Init) 必須在硬體初始化 (MX_USART2_UART_Init) **之後**，且在 OS 啟動 (osKernelStart) **之前** 呼叫，否則會操作到未初始化的指標 (NULL Pointer)。

### 專案進度 (Project Status)

- **軟體架構：** 已完成模組化，具備擴充性。
- **人機介面：** 已具備 CLI，未來可透過指令查詢感測器數值或設定參數。

---

## [Day 12] 2026-01-26：系統可靠度設計 (Watchdog Timer)

### 完成項目

1. **IWDG (獨立看門狗) 配置：**
    - 啟用 LSI (32kHz) 作為時鐘源。
    - 設定 Prescaler=32, Reload=3000，實現 **3秒** 超時重置機制。
2. **餵狗機制 (Keep Alive)：**
    - 在主任務迴圈中加入 HAL_IWDG_Refresh，確保系統正常運作時不會誤觸發。
3. **故障模擬 (Fault Injection)：**
    - 在 Shell 新增 bug 指令，刻意進入無窮迴圈且不餵狗，模擬系統當機 (System Freeze)。
4. **重啟原因分析 (Reset Cause Analysis)：**
    - 開機時讀取 RCC_CSR 暫存器旗標。
    - 成功區分 **正常上電 (POR/PDR)** 與 **看門狗重置 (IWDG Reset)**，並清除旗標。

### 技術筆記

- **為何選 IWDG 而非 WWDG？**
    - IWDG 使用內部低速時鐘 (LSI)，即使主時鐘 (HSE/HSI) 掛點，看門狗依然能運作，適合做最後一道防線。
- **自我修復 (Self-Healing)：**
    - 這是工業控制系統 (Industrial Control) 的必備功能，確保設備在無人值守時遇到干擾能自動恢復。

---

## [Day 13] 2026-01-27：中斷解耦與同步機制 (Binary Semaphore)

### 完成項目

1. **Binary Semaphore 配置：**
    - 在 FreeRTOS 中建立 myBinarySem01，作為 ISR 與 Task 之間的同步信號。
2. **推遲中斷處理 (Deferred Interrupt Processing) 實作：**
    - **ISR (生產者):** 修改 HAL_GPIO_EXTI_Callback，僅執行 osSemaphoreRelease (發送信號)，移除所有耗時邏輯，確保 ISR 快進快出。
    - **Task (消費者):** 建立 Button_Task，使用 osSemaphoreAcquire(..., osWaitForever) 進入阻塞狀態 (Blocked)。一旦收到信號立即喚醒執行業務邏輯 (如 LED 控制與 UART 傳輸)。
3. **架構優化：**
    - 成功將「硬體觸發」與「業務邏輯」解耦 (Decoupling)。解決了在中斷內無法使用 printf 或 Delay 的限制。

### 技術筆記

- **Semaphore vs. Mutex (面試重點)：**
    - **Mutex (互斥鎖):** 用於**資源保護** (如 UART 只有一個)。具有擁有權概念 (誰鎖的誰解開)，且支援優先級繼承 (Priority Inheritance) 防止優先級反轉。
    - **Binary Semaphore (二進位信號量):** 用於**任務同步** (如 ISR 通知 Task)。無擁有權概念 (ISR 給，Task 拿)，類似於一個只有 0 和 1 的旗標，但支援阻塞機制。
- **FromISR API 的必要性：**
    - FreeRTOS 區分 xSemaphoreGive 與 xSemaphoreGiveFromISR。
    - **原因 1 (不可阻塞):** ISR 沒有 TCB (Task Control Block)，不能進入 Block 狀態，因此 API 絕不能讓呼叫者睡覺。
    - **原因 2 (Context Switch):** ISR 結束時需檢查 pxHigherPriorityTaskWoken，若為真，需手動呼叫 portYIELD_FROM_ISR 觸發上下文切換，以降低延遲 (Latency)。
    - **CMSIS 封裝:** STM32 的 osSemaphoreRelease 內部已自動判斷執行環境 (xPortIsInsideInterrupt)，自動呼叫對應的 FreeRTOS API。

### 專案進度 (Project Status)

- **系統穩定性：** 大幅提升。按鈕連發不再會導致系統卡頓，因為運算壓力已轉移至 Task 層級排程。
- **RTOS 掌握度：** 已實作 Task, Queue, Software Timer, Binary Semaphore。

---

## 目前遇到的問題 (Current Issue: I2C ERROR 1)

**症狀：**

- 嘗試讀取 SHT30/AHT10 感測器時，HAL 庫回傳 **HAL_ERROR (1)**。
- I2C Scanner 無法掃描到任何裝置（直接 Error 跳出，而非 NACK）。

**已嘗試過的解決方案 (Debugging Attempts)：**

1. **軟體設定：**
    - 開啟 STM32 內部 GPIO Pull-up 電阻。
    - 調整 HAL_I2C_IsDeviceReady 的 Timeout 時間。
    - 確認 MX_I2C_Init() 有正確初始化。
    - 暫時關閉 Watchdog (IWDG) 避免掃描超時重置。
2. **硬體排錯：**
    - **交換線路：** SCL 與 SDA 對調測試。
    - **斷電重啟：** 拔除感測器電源重插，消除 Bus Stuck。
    - **更換腳位：**
        - 嘗試 **I2C1** (PB8/PB9)。
        - 嘗試 **I2C2** (PB10/PB3, D6/D3)。
    - **更換線材：** 更換杜邦線。

**暫定結論：**

在軟體配置正確、且更換過多組 STM32 腳位與線材的情況下，問題極大機率出在 **SHT30 感測器模組本身硬體故障** (如內部短路或 I2C 控制器損壞)。

---

### [Day 14] 2026-01-31：硬體除錯與電氣特性分析 (Physical Layer Debugging)

**狀況：** I2C 通訊持續失敗 (HAL_ERROR)，更換軟體參數無效。

**行動：**

1. **三用電表診斷：**
    - 測量 SCL/SDA 對 VCC 電阻：確認模組自帶 10kΩ 上拉電阻 (Pull-up)，排除 Open-Drain 浮接問題。
    - 測量連通性 (Continuity)：確認 MCU 腳位到感測器焊點導通，排除線路斷路。
2. **波形/時序分析 (推論)：**
    - 雖然電氣連接正常，但 HAL 庫回傳 Error 且 SR2 (Status Register) 顯示 Bus Busy 或無反應。
    - 嘗試手動發送 9 個 Clock Pulse 進行 Bus Recovery (解鎖)，無效。

**結論：** 硬體層 (Physical Layer) 連接正常，問題限縮於 **I2C 控制器異常** 或 **模組損壞**。

### [Day 15] 2026-02-01：暫存器檢視與位元模擬 (Low-Level Diagnosis)

**狀況：** HAL 庫無法驅動硬體，需確認是否為 Library Bug。

**行動：**

1. **Register Dump (暫存器轉儲)：**
    - 直接讀取 I2C1->CR1, SR1, SR2。
    - 發現 SR1 為 0x0000 (無 Start Bit 旗標)，SR2 無 BUSY 旗標。
    - **判定：** MCU 硬體周邊根本沒有送出訊號，可能是腳位衝突 (Pin Conflict) 或硬體單元故障。
2. **Bit-Banging (軟體模擬 I2C)：**
    - 嘗試用 GPIO Output 模擬 I2C 時序 (Software I2C)。
    - 結果依然 NACK。
    - **最終判定：** 高機率為 SHT30 感測器模組本身損壞 (Hardware Failure)。

**決策：** 停止無止盡的硬體除錯，轉向 **「軟體容錯設計 (Software Fault Tolerance)」**。

---

### [Gap] 2026-02-02 ~ 02-14：年節休假與專案重整 (Project Reset)

- **策略調整：** 專案目標從「驅動感測器」轉向為「設計高可靠度的嵌入式系統架構」。決定引入 **BSP (Board Support Package)** 與 **Mock Driver** 概念。

---

### [Day 16] 2026-02-17：軟體架構重構 (Architecture Refactoring)

**目標：** 實作硬體抽象層 (HAL)，解耦 (Decouple) 應用層與硬體層。

**完成項目：**

1. **定義介面 (Interface Definition)：**
    - 建立 bsp_sensor_if.h。
    - 定義 SensorDriver_t 結構體，包含 Init, ReadTemp, ReadHumi 等函式指標。
2. **實作虛擬驅動 (Mock Implementation)：**
    - 建立 bsp_sensor_mock.c。
    - 實作一個不依賴硬體 I2C 的驅動，回傳模擬的波動數據 (Sine wave / Random)。
        
        **技術筆記：**
        
    - 透過 Function Pointer 實現 C 語言的多型 (Polymorphism)。
    - 這使得上層 FreeRTOS Task 可以在沒有硬體的情況下繼續開發與測試 (Unit Testing)。

---

## [Day 17] 2026-02-18：硬體抽象層與虛擬驅動 (HAL & Mocking)

### 完成項目

1. **硬體抽象層 (HAL) 設計：**
    - 定義 bsp_sensor_if.h (Interface)，規範了 Init, ReadTemp 等標準介面。
    - 實作 bsp_sensor_mock.c (Implementation)，透過軟體演算法模擬真實感測器的行為 (產生 25~30°C 的波動數據)。
2. **C 語言物件導向 (OOP in C)：**
    - 使用 struct 包裝 **Function Pointers (函式指標)**，實現類似 C++ 的 **多型 (Polymorphism)**。
    - 主程式 (main.c) 透過統一指標 pSensor 呼叫功能，完全不依賴底層硬體實作，達成 **解耦 (Decoupling)**。
3. **Task 整合與驗證：**
    - 成功將 Mock Driver 掛載至 FreeRTOS 的 Sensor_Task。
    - 透過 UART 驗證了虛擬溫度的讀取與印出。

### 技術筆記 (Interview Key Points)

- **BSP (Board Support Package) 命名慣例：**
    - 將硬體相關程式碼隔離在 bsp_ 層，確保上層應用邏輯 (App Layer) 的可移植性。
- **const 關鍵字的雙重意義：**
    1. **安全性 (Safety)：** 防止程式在 Runtime 意外修改驅動指標，避免跳轉到非法位址 (Hard Fault)。
    2. **記憶體優化 (Optimization)：** 將結構體強制存放在 **Flash (ROM)** 而非 SRAM，節省寶貴的 RAM 空間。
- **浮點數陷阱：**
    - 嵌入式系統的 printf 往往預設不支援浮點數 (%f) 以節省空間。
    - 解決方案：將浮點數手動拆解為整數部分與小數部分 (int + int) 再印出。

### 除錯紀錄 (Troubleshooting)

- **邏輯錯誤 (Logic Error)：函式指標判斷**
    - **問題寫法：** if (pSensor->ReadTemp(&val) != NULL)
        - 這會直接**執行**函式，並檢查回傳值 (Result) 是否為 NULL。因 Mock 回傳 0 (Success)，導致判斷式恆為 False。
    - **修正寫法：** if (pSensor->ReadTemp != NULL)
        - 這才是檢查**函式是否存在** (Pointer Check)。
- **格式化錯誤 (Format Error)：**
    - 使用 %ld 列印 float 變數，導致二進位解釋錯誤印出亂碼。修正為轉型 (int) 後列印。

---

## [Day 18] 2026-02-19：介面整合與通訊除錯 (CLI Integration & UART Debugging)

### 完成項目

1. **跨模組資源共享 (extern 應用)：**
    - 在 my_shell.c 中使用 extern 關鍵字引用定義於 main.c 的 pSensor 全域指標。
    - 成功打通「使用者介面層 (Shell)」與「硬體驅動層 (BSP)」之間的通道，實現了分層架構的互動。
2. **指令功能實作 (temp 指令)：**
    - 修改 Shell 的指令解析邏輯，將 temp 指令重新導向至 pSensor->ReadTemp() 介面。
    - 現在輸入指令後，系統會回傳由 Mock Driver 產生的模擬溫度數據 (浮點數)，驗證了虛擬驅動的可用性。
3. **UART 通訊除錯 (CRLF Issue)：**
    - 解決了「輸入一次指令卻觸發兩次中斷（其中一次報錯）」的幽靈指令問題。
    - 深入分析終端機發送的 **Carriage Return (\r, 0x0D)** 與 **Line Feed (\n, 0x0A)** 行為差異。

### 技術筆記 (Interview Key Points)

- **extern 的正確用法：**
    - **定義 (Definition):** 變數實際佔用記憶體的地方 (在 main.c: const SensorDriver_t *pSensor = ...)。
    - **宣告 (Declaration):** 告訴編譯器「這個變數存在於別處」 (在 my_shell.c: extern const SensorDriver_t *pSensor;)。
    - **面試題：** 若未加 extern 直接在 header 定義變數，會導致 "Multiple Definition" 連結錯誤 (Linker Error)。
- **指標安全性檢查 (Pointer Safety)：**
    - 在呼叫 pSensor->ReadTemp() 之前，務必檢查 if (pSensor != NULL && pSensor->ReadTemp != NULL)。
    - 這能防止未來切換驅動或初始化失敗時，程式存取非法位址導致 **Hard Fault** 當機。
- **UART 換行符號陷阱：**
    - 常用的終端機軟體 (Putty, RealTerm) 按下 Enter 鍵時，通常會發送 \r\n (2 bytes)。
    - 若 Shell 程式邏輯是「收到換行就處理」，則會被觸發兩次：
        1. 收到 \r -> 解析指令 (成功)。
        2. 收到 \n -> 解析空字串 (失敗，顯示 Unknown Command)。

### 除錯紀錄 (Troubleshooting)

- **症狀：**
    - 輸入 temp 按下 Enter。
    - Console 輸出：codeText
        
        `Temp: 25.1 C
        Unknown Command`
        
- **根因分析 (Root Cause Analysis)：**
    - UART ISR (中斷服務程式) 將 \r 與 \n 視為兩次獨立的輸入結束訊號。
    - 第一次處理 temp (Buffer 內容正確)。
    - 第二次處理 \n (Buffer 已被清空或僅剩 \0)，導致 strcmp 比對失敗。
- **解決方案 (Solution)：**
    - 在指令解析器 (Parser) 前端加入「過濾器」。
    - 若 strlen(rx_buffer) == 0 或僅包含空白字元，則視為無效輸入直接忽略 (return/continue)，不進行錯誤回報。

### 專案狀態 (Status)

目前系統已具備：

1. **Mock Driver:** 模擬感測器數據。
2. **HAL Layer:** 抽換驅動不影響主程式。
3. **Robust Shell:** 穩定的命令列介面，已解決通訊雜訊問題。

---

## [Day 19] 2026-02-21：系統自我診斷功能 (System Diagnostics)

### 完成項目

1. **驅動程式元數據 (Driver Metadata)：**
    - 修改 bsp_sensor_if.h，在 SensorDriver_t 結構體中新增 const char *Name 欄位。
    - 在 bsp_sensor_mock.c 中填入識別字串 "Mock Sensor (Simulated)"，讓驅動程式具備自我描述能力。
2. **系統狀態指令 (status)：**
    - 在 Shell 中實作 status 指令，整合了 **驅動資訊** 與 **系統運行時間**。
    - **成果：** 輸入指令後，系統回傳如下報告，讓管理者能一眼確認目前是運行在「模擬模式」還是「真實硬體模式」。codeText
        - `-- SYSTEM STATUS ---
        Driver: Mock Sensor (Simulated)
        Uptime: 00:05:23--------------------`
3. **時間演算法實作：**
    - 利用 HAL_GetTick() 取得系統開機後的毫秒數 (Tick)。
    - 使用模數運算 (Modulo Operator %) 將毫秒轉換為人類可讀的 HH:MM:SS 格式。

### 技術筆記 (Interview Key Points)

- **自我描述架構 (Self-Describing Architecture)：**
    - 透過在物件 (Struct) 中嵌入 Metadata (如 .Name)，實現了 Runtime 的系統識別。
    - 這為未來切換 RealSensorDriver 預留了伏筆：切換驅動時，status 指令無需修改程式碼，即可自動顯示正確的硬體名稱。
- **時間管理的溢位風險 (Overflow Risk)：**
    - HAL_GetTick() 回傳 uint32_t，最大值約 49.7 天。
    - **面試題回答：** 對於 Side Project，49 天重置可接受；若為工業產品，應升級為 64-bit 變數或整合 RTC (Real Time Clock) 模組。
- **顯示緩衝區管理：**
    - 使用 sprintf 格式化多行字串時，需注意 char buffer[] 的大小分配，避免 Stack Overflow。

### 硬體故障分析 (Hardware Diagnosis)

**目前狀況：** I2C 通訊持續失敗，已更換過 **3 顆不同的感測器模組**，但症狀一致 (HAL_ERROR / Timeout)。

**推論 (Hypothesis)：**

1. **感測器損壞：** 機率極低 (已交叉驗證 3 顆)。
2. **線路配置：** 可能是上拉電阻 (Pull-up Resistor) 缺失或接觸不良。
3. **MCU 端口燒毀 (Pin Burnout)：** 高度懷疑 STM32 的 **PB8/PB9** 腳位內部的 Open-Drain 電晶體已損壞，導致無法正確拉低/拉高電位。

**驗證計畫 (Next Action)：**

- **GPIO Toggle Test：** 明日將撰寫一個簡單的 GPIO 翻轉程式，直接測量 PB8/PB9 對地電壓。
    - 若電壓無法在高低之間切換 -> **確認 MCU 腳位燒毀**。
    - 若電壓正常切換 -> **確認為 I2C 控制器或周邊電路問題**。

---

## [Day 20] 2026-02-22：硬體懸案破案與腳位重映射 (Hardware RCA & Pin Remapping)

### 完成項目 (Achievements)

1. **物理層徹底檢測 (Physical Layer Verification)：**
    - 撰寫 GPIO 翻轉測試程式，並使用三用電表直接測量 I2C 腳位電壓。
    - **發現：** PB9 (SDA / D14) 能夠正常輸出 0V 與 3.3V，但 PB8 (SCL / D15) 電壓毫無反應 (Dead Pin)。
2. **根本原因確認 (Root Cause Analysis, RCA)：**
    - 確診 MCU 內部的 PB8 腳位硬體電路已物理性燒毀。
    - 解釋了為何過去一週更換 3 顆感測器皆回傳 HAL_ERROR 或 Timeout：因時脈線 (SCL) 損壞，感測器無法接收 Clock 訊號，導致 I2C Bus 永久癱瘓。
3. **硬體重映射與修復 (Hardware Remapping & Workaround)：**
    - **軟體層：** 在 STM32CubeMX 中將損壞的 PB8 停用，利用 MCU 的多工器 (Multiplexer) 特性，將 I2C1_SCL 的 Alternate Function 重新映射 (Remap) 至備用腳位 PB6，並啟用內部上拉 (Pull-up)。
    - **硬體層：** 查閱 Nucleo-F411RE 腳位圖 (Pinout Diagram)，排除 A3 腳位的誤導 (實為 PB0)，正確定位出 PB6 對應的實體排針為 D10。
4. **I2C 掃描驗證 (Verification)：**
    - 將 SCL 實體線路改接至 D10 後，執行自製的 I2C Scanner。
    - **結果：** 成功掃描到 Found: 0x44 (SHT30 的預設 I2C 地址)！硬體正式宣告復活。

### 技術筆記 (Interview Key Points)

- **I2C 的心跳 (SCL) 與推輓/開汲極 (Push-Pull vs Open-Drain)：**
    - I2C 必須配置為 Open-Drain 並依賴上拉電阻。如果 SCL 腳位損壞無法拉低電位打拍子，整條 Bus 的資料 (SDA) 就會完全失去意義。
- **腳位重映射 (Pin Remapping / Alternate Function)：**
    - 在現代 ARM Cortex-M 微控制器中，周邊硬體 (Peripheral) 不是死綁在單一腳位上的。當遇到腳位衝突或燒毀時，透過修改暫存器切換 Alternate Function，是拯救系統的標準工程手段。
- **開發板絲印陷阱 (Board Silkscreen Trap)：**
    - Arduino 介面的絲印 (如 A3) 與 MCU 實際腳位 (PB0) 的對應關係極易混淆。排錯時必須**直接查閱官方 Datasheet 與 Pinout 圖**，不能單憑電表量測到 3.3V 就誤判 (Floating 腳位也可能有殘留電壓)。

### 專案狀態 (Project Status)

- **危機解除：** 困擾專案一週的硬體 I2C 故障已透過「腳位重映射」與「物理線路修正」完美解決。
- **系統架構準備就緒：** 我們的軟體層已經具備 BSP (Board Support Package) 架構，隨時可以迎接真實硬體的歸來。

---

## [Day 21] 2026-03-02：真實驅動實作與無縫架構整合 (Real Driver & Seamless Integration)

### 完成項目 (Achievements)

1. **研讀原廠手冊 (Datasheet Reading)：**
    - 獨立查閱 Sensirion SHT30 官方 Datasheet。
    - 提取出單次高精準度測量指令 (Single Shot, High Repeatability)：0x24, 0x00。
    - 解析出溫度換算公式：。
        
        ```
        T=−45+175×ST216−1T=−45+175×216−1ST
        ```
        
2. **實作硬體驅動層 (bsp_sensor_real.c)：**
    - 處理 STM32 I2C 7-bit 地址向左移位的陷阱 (0x44 << 1)。
    - 利用 HAL_I2C_Master_Transmit 發送指令，並透過 osDelay(15) 符合硬體量測所需的時序規範。
    - 利用 HAL_I2C_Master_Receive 讀取 6 Bytes 原始資料。
3. **位元運算與資料重組 (Bitwise Operations)：**
    - 透過移位與位元或運算 (rx_data[0] << 8) | rx_data[1]，成功將分離的高低 8-bit 資料重組成 16-bit 的有效無號整數 (uint16_t)。
4. **架構無縫切換 (Seamless Driver Swapping)：**
    - 在 main.c 中，僅修改一行指標指派 (pSensor = &RealSensorDriver)。
    - **成果：** 上層的 FreeRTOS Task 與 Shell CLI (status, temp 指令) **完全無需修改任何程式碼**，即成功從「虛擬模擬數據」切換為「真實環境溫度」，完美驗證了 BSP 分層架構與軟體解耦 (Decoupling) 的巨大優勢。

### 技術筆記 (Interview Key Points)

- **物件導向思維在 C 語言的實踐 (OOP in C)：**
    - 這次的成功切換，是面試時展現「軟體工程能力」的最佳案例。透過 SensorDriver_t 介面，應用程式端不需要知道底層是 I2C、SPI 還是 Mock，只管呼叫 ReadTemp。這大幅提升了程式碼的「可維護性」與「可測試性」。
- **硬體時序控制 (Hardware Timing Constraints)：**
    - 感測器收到測量指令後需要時間進行 ADC 轉換 (SHT30 約需 15ms)。若不加 Delay 直接讀取，將導致 I2C NACK 或讀到垃圾數據。這展現了工程師對硬體特性的掌握。

---

## [Day 22] 2026-03-04：高可靠度容錯設計與黑盒子日誌架構 (Fault Tolerance & Logger Architecture)

### 完成項目 (Achievements)

1. **硬體拔線破壞測試 (Chaos Engineering & Fault Injection)：**
    - 於 Runtime 執行期間，物理性拔除 SHT30 感測器之 I2C 腳位，模擬工業現場設備損壞、線路脫落之極端情境。
    - **成果：** 系統未發生死鎖 (Deadlock) 或 Hard Fault。系統精準捕捉 HAL_ERROR，並在 CLI 介面印出錯誤警告。
2. **優雅降級與熱容錯機制 (Graceful Degradation)：**
    - 實作「錯誤計數器 (Error Counter)」狀態機。
    - 當 I2C 連續 3 次 Timeout 讀取失敗，觸發安全防護網 (Fallback Mechanism)。
    - 系統自動將底層全域指標 pSensor 從 RealSensorDriver 重新導向至 MockSensorDriver，維持主系統與 RTOS Task 穩定運行，達成軟體層級的自我修復 (Self-Healing)。
3. **黑盒子日誌資料結構設計 (Black Box Logger Data Structure)：**
    - 設計 LogRecord_t 結構體，利用 int8_t (溫度) 與 enum (驅動狀態) 取代浮點數與指標，極大化節省 Flash 儲存空間。
    - 導入 **記憶體對齊 (Memory Alignment)** 概念，利用 reserved[2] 將單筆 Log 湊滿 8 Bytes，優化 Flash 寫入效率。
4. **環形緩衝區實作 (Ring Buffer Implementation)：**
    - 撰寫 Add_Log() 函式，於 SRAM 中建立暫存陣列 (LOG_MAX_COUNT = 10)。
    - 實作 Index 循環歸零機制，避免陣列越界，並作為未來 Flash 批次寫入 (Batch Write) 的觸發條件。

### 技術筆記 (Interview Key Points)

- **Flash 磨損平均與寫入策略 (Wear Leveling & Flash Lifespan)：**
    - MCU 內建之 Flash 具備物理擦寫壽命限制 (約 10,000 次 P/E Cycles)。若採用逐筆高頻寫入，將導致 Sector 快速損壞。
    - **解決方案：** 導入 RAM Ring Buffer，累積滿 10 筆資料 (或發生 Critical Error 時) 再執行一次 Flash 寫入，將 Flash 壽命延長數十倍，符合工業產品設計規範。
- **C 語言野指標陷阱 (Wild Pointer Trap)：**
    - 宣告結構體指標 (LogRecord_t *p) 時，若未賦予實體記憶體位址 (如 malloc 或指向現有變數 &buffer)，直接賦值將導致非法記憶體存取 (Hard Fault)。在開發 Add_Log 時透過 Code Review 排除此潛在危機。
- **持久化資料的安全規範 (Data Persistence Safety)：**
    - 存入 Flash 的資料**絕對不可包含指標 (Pointers)**。因韌體更新 (OTA) 或重新編譯後，函式或變數之實體記憶體位址將會偏移。需改用數值或列舉型別 (Enum) 來記錄狀態。

---

## [Day 23] 2026-03-06：掉電保存與記憶體映射 (Flash Persistence & Memory Mapping)

### 完成項目 (Achievements)

1. **Flash 驅動層實作 (my_flash.c)：**
    - 完成 Save_To_Flash 函式，整合了 HAL_FLASH_Unlock, FLASH_Erase (Sector 7), HAL_FLASH_Program 等標準流程。
    - 解決了 32-bit Word 寫入與結構體大小不對齊的問題，確保資料完整寫入。
2. **日誌寫入邏輯 (Add_Log in my_logger.c)：**
    - 實作 Ring Buffer 滿載觸發機制。當 RAM 中的 10 筆暫存資料滿了，自動觸發 Flash 寫入。
    - 使用 **強制轉型 (Type Casting)** 技巧，將 LogRecord_t 結構體陣列轉換為 uint32_t * 指標傳遞給 Flash 驅動。
3. **記憶體映射讀取 (Log_Dump)：**
    - 利用 STM32 的 **Memory Mapped** 特性，直接將 Flash Sector 7 的絕對地址 (0x08060000) 強制轉型為 LogRecord_t * 指標。
    - 透過指標運算 (Pointer Arithmetic) 直接以陣列方式 (pFlashLog[i]) 讀取歷史資料，無需透過額外的 API 搬運，大幅提升讀取效率。
4. **整合測試 (System Integration Test)：**
    - 執行「斷電測試」：在系統運作並觸發寫入後，按下 Reset 鍵模擬斷電。
    - 透過 Shell 輸入 history 指令，成功撈回斷電前的溫度與驅動狀態數據，驗證了系統的資料持久化能力 (Persistence)。

### 技術筆記 (Interview Key Points)

- **記憶體映射 I/O (Memory Mapped I/O)：**
    - 在 ARM 架構中，Flash 與 RAM 統一編址。因此讀取 Flash 不需要像讀取硬碟那樣經過複雜的 Driver，而是可以直接用「指標」去指。
    - **關鍵程式碼：** LogRecord_t *pLog = (LogRecord_t *)ADDR;。這行程式碼展現了對電腦組織結構的深刻理解。
- **指標運算的本質：**
    - 當指標型別被定義為結構體 (LogRecord_t *)，編譯器會自動依據結構體的大小 (Sizeof) 進行偏移量計算。pLog[1] 會自動跳過 8 Bytes (結構體大小)，而非 1 Byte。這比使用 Linked List 更節省空間且存取速度更快 (O(1) vs O(n))。
- **資料序列化 (Serialization)：**
    - 雖然我們直接將結構體寫入 Flash (Raw Binary)，但在實務上需注意 **Endianness (大小端模式)** 與 **Structure Padding (結構體填充)**。本專案透過手動加入 reserved 欄位確保 8-byte 對齊，避免了跨平台讀取的問題。

### 專案狀態 (Project Status)

- **Feature Complete (功能開發完成)：** 所有核心模組 (RTOS, BSP, Shell, Flash) 皆已實作並驗證通過。
- **系統完整度：** 具備「即時監控」、「故障容錯」、「歷史回溯」三大工業級特徵。
