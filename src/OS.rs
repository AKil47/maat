use windows::Win32::{
    Foundation::{BOOL, FALSE, LPARAM, RECT, TRUE},
    Graphics::Gdi::{EnumDisplayMonitors,HDC, HMONITOR, MONITORINFO, GetMonitorInfoW}
};
use std::mem;

/// Get "handle" to each connected display
/// Each HMONITOR is a special struct that can be used with Windows APIs to get screen data
/// This also includes virtual displays though
/// 
pub fn get_screens() -> Vec<HMONITOR> {
    let mut monitors: Vec<HMONITOR> = Vec::new();

    unsafe extern "system" fn monitor_enum_proc(
        hmonitor: HMONITOR,
        _hdc: HDC,
        _lprc: *mut RECT,
        dwdata: LPARAM,
    ) -> BOOL {
        let monitors = &mut *(mem::transmute::<LPARAM, *mut Vec<HMONITOR>>(dwdata));
        monitors.push(hmonitor);
        TRUE
    }

    unsafe {
        EnumDisplayMonitors(
            None,
            None,
            Some(monitor_enum_proc),
            LPARAM(&mut monitors as *mut Vec<HMONITOR> as isize),
        );
    }

    monitors
}

/// Gets monitor size from handle
/// 
/// # Arguments
/// 
/// * `monitor` - HMONITOR handle that represents monitor
/// 
pub fn get_monitor_size(monitor: HMONITOR) -> Result<(u32, u32), ()> {
    let mut monitor_info: MONITORINFO = unsafe { std::mem::zeroed() };
    monitor_info.cbSize = std::mem::size_of::<MONITORINFO>() as u32;

    // Call GetMonitorInfoW to retrieve the monitor information
    let success = unsafe { GetMonitorInfoW(monitor, &mut monitor_info as *mut _) };

    if success != FALSE {
        // Extract the screen width and height from the monitor information
        let width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
        let height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
        Ok((width as u32, height as u32))
    } else {
        println!("{:?}", success);
        Err(()) // Handle error if GetMonitorInfoW fails
    }
}