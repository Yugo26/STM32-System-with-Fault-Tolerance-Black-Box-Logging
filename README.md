System Architecture

為了確保降低硬體依賴性，本專案採用"高內聚、低耦合"的分層架構設計，為了大幅提升程式碼的可移植性與可測試性：

*   **Application 層**：`main` 負責系統初始化與 FreeRTOS Task 管理。
*   **Modules 層**：實作 `my_shell` (CLI 介面) 與 `my_logger` (黑盒子日誌)，完全不依賴特定硬體。
*   **BSP (Board Support Package) 層**：統一感測器操作介面，實作真實感測器 (`bsp_sensor_real`) 與虛擬驅動 (`bsp_sensor_mock`)。
*   **HAL 層**：ST 官方硬體抽象層。
*   **Hardware 層**：STM32F411RE 與 SHT30。

Key Features

### 1. 硬體除錯與軟體重映射 (Hardware Troubleshooting & Pin Remapping)
*   **情境**：開發初期在硬體未到齊前，先行開發 `I2C_Scanner`。實體感測器接入後發現無法尋址，且切換至 I2C2 亦無效。
*   **排查與解決**：不侷限於純軟體 Debug，透過三用電表實測腳位電氣特性，確認原 I2C 腳位發生物理性燒毀（無法正常拉高/拉低）。為挽救硬體，透過軟體配置將 I2C 腳位重映射 (Remap) 至 PB6/PB7，成功恢復通訊，展現跨軟硬體的 Trouble-shooting 能力。

### 2. 熱容錯與虛擬驅動切換 (Graceful Degradation & Mock Driver)
（這裡留給你填空！你打算怎麼描述你在影片裡展示的「拔掉線也不會當機，並自動切換到 Mock Driver」的設計？）
