//
// Created by carlo on 2024-09-22.
//

#ifndef SURFACE_HPP

namespace engine
{

    static vk::UniqueSurfaceKHR create_win32_surface(vk::Instance instance, window_desc window_desc)
    {

        auto surface_create_info = vk::Win32SurfaceCreateInfoKHR()
        .setHwnd(window_desc.h_wnd)
        .setHinstance(window_desc.h_instance);
        return instance.createWin32SurfaceKHRUnique(surface_create_info);
    } 
    
}
#define SURFACE_HPP

#endif //SURFACE_HPP
