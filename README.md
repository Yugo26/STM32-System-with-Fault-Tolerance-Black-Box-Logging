System Architecture

為了確保降低硬體依賴性，本專案採用"高內聚、低耦合"的分層架構設計，為了大幅提升程式碼的可移植性與可測試性：

   Application 層：`main` 負責系統初始化與 FreeRTOS Task 管理。
   Modules 層：實作 `my_shell` (CLI 介面) 與 `my_logger` (黑盒子日誌)，完全不依賴特定硬體。
   **BSP (Board Support Package) 層**：統一感測器操作介面，實作真實感測器 (`bsp_sensor_real`) 與虛擬驅動 (`bsp_sensor_mock`)。
*   **HAL 層**：ST 官方硬體抽象層。
*   **Hardware 層**：STM32F411RE 與 SHT30。

Key Features

#### 1. 硬體除錯與軟體重映射
   情況：因為在開發初期硬體還購買齊全，所以先開發 `I2C_Scanner`。但實體感測器接入後發現無法尋址，且切換至 I2C2 也無效。
   排查與解決：不只有嘗試純軟體 Debug，另外透過三用電表實測腳位電氣特性，確認原 I2C 腳位發生物理性燒毀（無法正常拉高/拉低）。為挽救硬體，透過軟體配置將 I2C 腳位重映射至 PB6/PB9，便成功恢復通訊。

#### 2. 熱容錯與虛擬驅動切換
   設計初衷：因為在開發階段遭遇 I2C 硬體異常，為了不中斷上層應用程式的開發，引入了 Mock Driver 進行軟硬體解耦。
   容錯機制：在底層設計中加入了狀態監控。當真實感測器連續發生 3 次 I2C Timeout 時，系統不會觸發 HardFault 或死鎖，而是自動將底層的資料指標切換至 Mock Driver。這確保了系統在特殊情況下發生硬體斷線時還是能維持基本的系統運作與介面回應，展現高可靠度的設計。

#### 3. 黑盒子日誌與記憶體直接讀取
   實作機制：因考量到 Flash 寫入時間較長且需以 Sector 抹除，直接寫入會阻塞 RTOS Task。因此設計 RAM Ring Buffer 進行快取，等 Buffer 滿了或特定條件時才寫入 Internal Flash (Sector 7)。
   除錯經驗：開發初期曾因 "開機即執行 Flash 抹除"，導致 CPU 卡死且 SWD 介面失效（板子鎖死）。後續透過 STM32CubeProgrammer 進行底層硬體 Reset 成功救回，並藉此深刻理解 Flash 操作對 CPU 總線根中斷的影響，進而完善了 Flash 操作的時序與安全防護機制。
   效能優化：讀取歷史紀錄時，善用 ARM Cortex-M 架構特性，直接透過 Memory Mapped 方式以指標存取 Flash 位址，省去額外的 RAM 讀取搬移開銷。
