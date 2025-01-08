//
// Created by carlo on 2024-09-22.
//

#ifndef INSTANCE_HPP

namespace ENGINE
{

    static vk::UniqueSurfaceKHR CreateWin32Surface(vk::Instance instance, WindowDesc windowDesc)
    {

        auto surfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR()
        .setHwnd(windowDesc.hWnd)
        .setHinstance(windowDesc.hInstance);
        return instance.createWin32SurfaceKHRUnique(surfaceCreateInfo); 
    } 
    
}
#define INSTANCE_HPP

#endif //INSTANCE_HPP
