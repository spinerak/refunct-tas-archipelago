use std::cell::Cell;
use std::fmt::{Formatter, Pointer};
use std::ops::Deref;
use std::sync::Mutex;
use std::ptr;
use crate::native::{ArrayWrapper, ObjectIndex, StructValueWrapper, UeObjectWrapperType, UeScope, UObject, ObjectWrapper, ClassWrapper, UWorld, DynamicValue, BoolValueWrapper, DerefToObjectWrapper, AMyCharacter};
use crate::native::reflection::{AActor, ActorWrapper, UeObjectWrapper};
use crate::native::ue::{FRotator, FVector, FName};
use crate::native::uworld::{ESpawnActorCollisionHandlingMethod, ESpawnActorNameMode, FActorSpawnParameters};

pub static LEVELS: Mutex<Vec<Level>> = Mutex::new(Vec::new());

pub struct Level {
    pub level: ObjectIndex<LevelWrapperType>,
    pub platforms: Vec<ObjectIndex<PlatformWrapperType>>,
    pub cubes: Vec<ObjectIndex<CubeWrapperType>>,
    pub buttons: Vec<ObjectIndex<ButtonWrapperType>>,
    pub lifts: Vec<ObjectIndex<LiftWrapperType>>,
    pub pipes: Vec<ObjectIndex<PipeWrapperType>>,
    pub springpads: Vec<ObjectIndex<SpringpadWrapperType>>,
}

#[derive(Debug, Clone)]
pub struct LevelWrapper<'a> {
    base: ActorWrapper<'a>,
}
pub enum LevelWrapperType {}
impl UeObjectWrapperType for LevelWrapperType {
    type UeObjectWrapper<'a> = LevelWrapper<'a>;
}
unsafe impl<'a> UeObjectWrapper<'a> for LevelWrapper<'a> {
    type UeObjectWrapperType = LevelWrapperType;
    type Wrapping = AActor;
    const CLASS_NAME: &'static str = "BP_LevelRoot_C";

    unsafe fn create(ptr: *mut Self::Wrapping) -> LevelWrapper<'a> {
        LevelWrapper::new(ActorWrapper::new(ptr))
    }
}
impl<'a> Deref for LevelWrapper<'a> {
    type Target = ActorWrapper<'a>;

    fn deref(&self) -> &Self::Target {
        &self.base
    }
}
impl<'a> Pointer for LevelWrapper<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        Pointer::fmt(&self.base, f)
    }
}

impl<'a> LevelWrapper<'a> {
    pub fn new(level: ActorWrapper<'a>) -> LevelWrapper<'a> {
        assert_eq!(level.class().name(), "BP_LevelRoot_C");
        LevelWrapper { base: level }
    }
    pub fn level_index(&self) -> usize {
        self.base.get_field("LevelIndex").unwrap::<i32>().try_into().unwrap()
    }
    pub fn source_location(&self) -> (f32, f32, f32) {
        let loc = self.base.get_field("SourcePosition").unwrap::<StructValueWrapper>();
        (loc.get_field("X").unwrap(), loc.get_field("Y").unwrap(), loc.get_field("Z").unwrap())
    }
    pub fn set_source_location(&self, x: f32, y: f32, z: f32) {
        let loc = self.base.get_field("SourcePosition").unwrap::<StructValueWrapper>();
        loc.get_field("X").unwrap::<&Cell<f32>>().set(x);
        loc.get_field("Y").unwrap::<&Cell<f32>>().set(y);
        loc.get_field("Z").unwrap::<&Cell<f32>>().set(z);
    }
    pub fn platforms(&self) -> impl Iterator<Item = PlatformWrapper<'a>> + '_ {
        self.base.get_field("FertileLands").unwrap::<ArrayWrapper<'_, _>>()
            .into_iter()
    }
    pub fn _platform(&self, index: usize) -> Option<PlatformWrapper<'a>> {
        let array = self.base.get_field("FertileLands").unwrap::<ArrayWrapper<'_, _>>();
        array.get(index)
    }
    pub fn cubes(&self) -> impl Iterator<Item = CubeWrapper<'a>> + '_ {
        self.base.get_field("Collectibles").unwrap::<ArrayWrapper<'_, _>>()
            .into_iter()
    }
    pub fn _cube(&self, index: usize) -> Option<CubeWrapper<'a>> {
        let array = self.base.get_field("Collectibles").unwrap::<ArrayWrapper<'_, _>>();
        array.get(index)
    }
    pub fn buttons(&self) -> impl Iterator<Item = ButtonWrapper<'a>> + '_ {
        self.base.get_field("Buttons").unwrap::<ArrayWrapper<'_, _>>()
            .into_iter()
    }
    pub fn _button(&self, index: usize) -> Option<ButtonWrapper<'a>> {
        let array = self.base.get_field("Buttons").unwrap::<ArrayWrapper<'_, _>>();
        array.get(index)
    }
    pub fn speed(&self) -> f32 {
        self.base.get_field("Speed").unwrap()
    }
    pub fn set_speed(&self, speed: f32) {
        self.base.get_field("Speed").unwrap::<&Cell<f32>>().set(speed)
    }
}

#[derive(Debug, Clone)]
pub struct PlatformWrapper<'a> {
    base: ActorWrapper<'a>,
}
pub enum PlatformWrapperType {}
impl UeObjectWrapperType for PlatformWrapperType {
    type UeObjectWrapper<'a> = PlatformWrapper<'a>;
}
unsafe impl<'a> UeObjectWrapper<'a> for PlatformWrapper<'a> {
    type UeObjectWrapperType = PlatformWrapperType;
    type Wrapping = AActor;
    const CLASS_NAME: &'static str = "BP_IslandChunk_C";

    unsafe fn create(ptr: *mut Self::Wrapping) -> PlatformWrapper<'a> {
        PlatformWrapper::new(ActorWrapper::new(ptr))
    }
}
impl<'a> Deref for PlatformWrapper<'a> {
    type Target = ActorWrapper<'a>;

    fn deref(&self) -> &Self::Target {
        &self.base
    }
}
impl<'a> Pointer for PlatformWrapper<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        Pointer::fmt(&self.base, f)
    }
}
impl<'a> PlatformWrapper<'a> {
    pub fn new(base: ActorWrapper<'a>) -> PlatformWrapper<'a> {
        assert_eq!(base.class().name(), "BP_IslandChunk_C");
        PlatformWrapper { base }
    }
}
#[derive(Debug, Clone)]
pub struct CubeWrapper<'a> {
    base: ActorWrapper<'a>,
}

impl<'a> CubeWrapper<'a> {
    pub fn spawn(x: f32, y: f32, z: f32) -> Result<CubeWrapper<'a>, &'static str> {
        UeScope::with(|scope| unsafe {
            // Find the BP_PowerCore_C class
            let cube_class = scope.iter_global_object_array()
                .map(|item| item.object())
                .find(|obj| {
                    obj.class().name() == "BlueprintGeneratedClass"
                        && obj.name() == "BP_PowerCore_C"
                })
                .ok_or("Could not find BP_PowerCore_C class")?;

            let class: ClassWrapper = cube_class.upcast();

            let location = FVector { x, y, z };
            let rotation = FRotator { pitch: 0.0, yaw: 0.0, roll: 0.0 };
            let spawn_parameters = FActorSpawnParameters {
                name: FName::NAME_None,
                template: ptr::null(),
                owner: ptr::null(),
                instigator: ptr::null(),
                override_level: ptr::null(),
                spawn_collision_handling_override: ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
                bitfield: FActorSpawnParameters::B_NO_FAIL,
                name_node: ESpawnActorNameMode::RequiredFatal,
                object_flags: 0x00000000,
            };

            let actor_ptr = UWorld::spawn_actor(
                class.as_ptr(),
                &location,
                &rotation,
                &spawn_parameters,
            );

            if actor_ptr.is_null() {
                return Err("SpawnActor returned null");
            }

            let cube = CubeWrapper::new(ActorWrapper::new(actor_ptr));
            cube.play_animation();

            // Mark as root to prevent GC
            let item = scope.object_array().get_item_of_object(&cube);
            item.mark_as_root_object(true);

            Ok(cube)
        })
    }

    pub fn _destroy(&self) {
        unsafe {
            UWorld::destroy_actor(self.base.as_ptr() as *const AActor, true, true);
        }
    }

    pub fn reset(&self) {
        self.set_color(1.0, 0.016666, 0.03305); // colors used by the base game
        self.set_scale(1.0);
        self.set_picked_up(false);
        self.set_hidden(false);
    }

    pub fn set_hidden(&self, hidden: bool) {
        let set_hidden = self.base.class()
            .find_function("SetActorHiddenInGame")
            .unwrap();

        let params = set_hidden.create_argument_struct();
        params.get_field("bNewHidden").unwrap::<BoolValueWrapper>().set(hidden);

        unsafe { set_hidden.call(self.base.as_ptr(), &params); }
    }

    pub fn is_picked_up(&self) -> bool {
        self.base.get_field("IsPickedUp").unwrap::<BoolValueWrapper>()._get()
    }

    pub fn set_picked_up(&self, picked_up: bool) {
        self.base.get_field("IsPickedUp").unwrap::<BoolValueWrapper>().set(picked_up);
    }

    pub fn pickup(&self) {
        self.set_collision(true);

        // Trigger the collision event
        let trigger_fn = self.class()
            .find_function("BndEvt__Trigger_K2Node_ComponentBoundEvent_24_ComponentBeginOverlapSignature__DelegateSignature")
            .unwrap();
        let params = trigger_fn.create_argument_struct();
        let movement = unsafe { ObjectWrapper::new(AMyCharacter::get_player().movement() as *mut UObject) };
        let updated_primitive = movement.get_field("UpdatedPrimitive").unwrap::<ObjectWrapper>();
        let trigger = self.get_field("Trigger").unwrap::<ObjectWrapper>();
        params.get_field("OverlappedComponent").set_object(&updated_primitive);
        params.get_field("OtherComp").set_object(&trigger);
        params.get_field("OtherActor").set_object(&self);
        params.get_field("OtherBodyIndex").unwrap::<&Cell<i32>>().set(0);
        unsafe {
            trigger_fn.call(self.as_ptr(), &params);
        }

        self.set_picked_up(true);
        self.set_hidden(true);
    }

    pub fn set_collision(&self, enabled: bool) {
        AActor::set_actor_enable_collision(self.base.as_ptr(), enabled);
    }

    pub fn set_color(&self, r: f32, g: f32, b: f32) {
        self.set_mesh_color(r, g, b);
        self.set_light_color(r, g, b);
    }

    fn set_mesh_color(&self, r: f32, g: f32, b: f32) {
        let mesh: ObjectWrapper = self.base.get_field("Mesh").unwrap();

        let create_dynamic = mesh.class()
            .find_function("CreateDynamicMaterialInstance")
            .unwrap();

        let params = create_dynamic.create_argument_struct();
        params.get_field("ElementIndex").unwrap::<&Cell<i32>>().set(0);
        unsafe { create_dynamic.call(mesh.as_ptr(), &params); }

        let dynamic_mat: ObjectWrapper = params.get_field("ReturnValue").unwrap();

        let set_vector = dynamic_mat.class()
            .find_function("SetVectorParameterValue")
            .unwrap();

        let arg_struct = set_vector.create_argument_struct();

        let fname = FName::from("GlowColor");
        unsafe {
            let base_ptr = arg_struct.as_ptr() as *mut u8;
            let name_field = arg_struct.get_field("ParameterName");
            let offset = name_field.prop().offset();
            let fname_ptr = base_ptr.offset(offset) as *mut FName;
            ptr::write(fname_ptr, fname);
        }

        let value: StructValueWrapper = arg_struct.get_field("Value").unwrap();
        value.get_field("R").unwrap::<&Cell<f32>>().set(r * 20.0);
        value.get_field("G").unwrap::<&Cell<f32>>().set(g * 20.0);
        value.get_field("B").unwrap::<&Cell<f32>>().set(b * 20.0);
        value.get_field("A").unwrap::<&Cell<f32>>().set(1.0);

        unsafe { set_vector.call(dynamic_mat.as_ptr(), &arg_struct); }
    }

    fn set_light_color(&self, r: f32, g: f32, b: f32) {
        if let Some(light) = self.base.get_field("PointLight1").unwrap_nullable::<ObjectWrapper>() {
            let set_light_color = light.class()
                .find_function("SetLightColor")
                .unwrap();

            let params = set_light_color.create_argument_struct();
            let color: StructValueWrapper = params.get_field("NewLightColor").unwrap();

            color.get_field("R").unwrap::<&Cell<f32>>().set(r);
            color.get_field("G").unwrap::<&Cell<f32>>().set(g);
            color.get_field("B").unwrap::<&Cell<f32>>().set(b);
            color.get_field("A").unwrap::<&Cell<f32>>().set(1.0);

            unsafe {
                set_light_color.call(light.as_ptr(), &params);
            }
        } else {
            log!("Warning: No light component found on cube {}", self.as_ptr().addr());
        }
    }

    pub fn set_scale(&self, new_scale: f32) {
        let old_bounds = self.get_actor_bounds();
        let old_z = old_bounds.2;
        let old_h = old_bounds.5;

        let root_component: ObjectWrapper = self.get_field("RootComponent").unwrap();
        let set_world_scale = root_component.class().find_function("SetRelativeScale3D").unwrap();
        let params = set_world_scale.create_argument_struct();
        let s: StructValueWrapper = params.get_field("NewScale3D").unwrap();
        s.get_field("X").unwrap::<&Cell<f32>>().set(new_scale);
        s.get_field("Y").unwrap::<&Cell<f32>>().set(new_scale);
        s.get_field("Z").unwrap::<&Cell<f32>>().set(new_scale);

        unsafe { set_world_scale.call(root_component.as_ptr(), &params); }

        // The anchor for the cube mesh is below the cube, so when we scale up, the cube moves up.
        // So we need to move the position back down to compensate.
        let new_bounds = self.get_actor_bounds();
        let new_z = new_bounds.2;
        let new_h = new_bounds.5;

        let loc = self.absolute_location();
        self.set_location(loc.0, loc.1, loc.2 - (new_z - old_z) + (new_h - old_h)/2.0);
    }

    pub fn set_location(&self, x: f32, y: f32, z: f32) {
        let root_component: ObjectWrapper = self.get_field("RootComponent").unwrap();
        let set_world_location_and_rotation = root_component.class().find_function("K2_SetWorldLocation").unwrap();

        let params = set_world_location_and_rotation.create_argument_struct();
        let location: StructValueWrapper = params.get_field("NewLocation").unwrap();
        location.get_field("X").unwrap::<&Cell<f32>>().set(x);
        location.get_field("Y").unwrap::<&Cell<f32>>().set(y);
        location.get_field("Z").unwrap::<&Cell<f32>>().set(z);
        params.get_field("bSweep").unwrap::<BoolValueWrapper>().set(false);
        params.get_field("bTeleport").unwrap::<BoolValueWrapper>().set(true);
        unsafe {
            set_world_location_and_rotation.call(root_component.as_ptr(), &params);
        }
    }

    pub fn play_animation(&self) {
        let timeline: ObjectWrapper = self.base.get_field("MoveAnimation").unwrap();

        // Set play rate to match original cubes (0.25)
        let the_timeline: StructValueWrapper = timeline.get_field("TheTimeline").unwrap();
        the_timeline.get_field("PlayRate").unwrap::<&Cell<f32>>().set(0.25);

        // Find and call the Play function
        if let Some(play_func) = timeline.class().find_function("Play") {
            let params = play_func.create_argument_struct();
            unsafe {
                play_func.call(timeline.as_ptr(), &params);
            }
            log!("Started MoveAnimation timeline");
        } else {
            log!("Warning: Could not find Play function on timeline");
        }
    }
}

pub enum CubeWrapperType {}
impl UeObjectWrapperType for CubeWrapperType {
    type UeObjectWrapper<'a> = CubeWrapper<'a>;
}
unsafe impl<'a> UeObjectWrapper<'a> for CubeWrapper<'a> {
    type UeObjectWrapperType = CubeWrapperType;
    type Wrapping = AActor;
    const CLASS_NAME: &'static str = "BP_PowerCore_C";

    unsafe fn create(ptr: *mut Self::Wrapping) -> CubeWrapper<'a> {
        CubeWrapper::new(ActorWrapper::new(ptr))
    }
}
impl<'a> Deref for CubeWrapper<'a> {
    type Target = ActorWrapper<'a>;

    fn deref(&self) -> &Self::Target {
        &self.base
    }
}
impl<'a> Pointer for CubeWrapper<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        Pointer::fmt(&self.base, f)
    }
}
impl<'a> CubeWrapper<'a> {
    pub fn new(cube: ActorWrapper<'a>) -> CubeWrapper<'a> {
        assert_eq!(cube.class().name(), "BP_PowerCore_C");
        CubeWrapper { base: cube }
    }
}
#[derive(Debug, Clone)]
pub struct ButtonWrapper<'a> {
    base: ActorWrapper<'a>,
}
pub enum ButtonWrapperType {}
impl UeObjectWrapperType for ButtonWrapperType {
    type UeObjectWrapper<'a> = ButtonWrapper<'a>;
}
unsafe impl<'a> UeObjectWrapper<'a> for ButtonWrapper<'a> {
    type UeObjectWrapperType = ButtonWrapperType;
    type Wrapping = AActor;
    const CLASS_NAME: &'static str = "BP_Button_C";

    unsafe fn create(ptr: *mut Self::Wrapping) -> ButtonWrapper<'a> {
        ButtonWrapper::new(ActorWrapper::new(ptr))
    }
}
impl<'a> Deref for ButtonWrapper<'a> {
    type Target = ActorWrapper<'a>;

    fn deref(&self) -> &Self::Target {
        &self.base
    }
}
impl<'a> Pointer for ButtonWrapper<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        Pointer::fmt(&self.base, f)
    }
}
impl<'a> ButtonWrapper<'a> {
    pub fn new(button: ActorWrapper<'a>) -> ButtonWrapper<'a> {
        assert_eq!(button.class().name(), "BP_Button_C");
        ButtonWrapper { base: button }
    }
}
#[derive(Debug, Clone)]
pub struct LiftWrapper<'a> {
    base: ActorWrapper<'a>,
}
pub enum LiftWrapperType {}
impl UeObjectWrapperType for LiftWrapperType {
    type UeObjectWrapper<'a> = LiftWrapper<'a>;
}
unsafe impl<'a> UeObjectWrapper<'a> for LiftWrapper<'a> {
    type UeObjectWrapperType = LiftWrapperType;
    type Wrapping = AActor;
    const CLASS_NAME: &'static str = "BP_Lift_C";

    unsafe fn create(ptr: *mut Self::Wrapping) -> Self {
        LiftWrapper::new(ActorWrapper::new(ptr))
    }
}
impl<'a> Deref for LiftWrapper<'a> {
    type Target = ActorWrapper<'a>;

    fn deref(&self) -> &Self::Target {
        &self.base
    }
}
impl<'a> Pointer for LiftWrapper<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        Pointer::fmt(&self.base, f)
    }
}
impl<'a> LiftWrapper<'a> {
    pub fn new(lift: ActorWrapper<'a>) -> LiftWrapper<'a> {
        assert_eq!(lift.class().name(), "BP_Lift_C");
        LiftWrapper { base: lift }
    }
}
#[derive(Debug, Clone)]
pub struct PipeWrapper<'a> {
    base: ActorWrapper<'a>,
}
pub enum PipeWrapperType {}
impl UeObjectWrapperType for PipeWrapperType {
    type UeObjectWrapper<'a> = PipeWrapper<'a>;
}
unsafe impl<'a> UeObjectWrapper<'a> for PipeWrapper<'a> {
    type UeObjectWrapperType = PipeWrapperType;
    type Wrapping = AActor;
    const CLASS_NAME: &'static str = "BP_TravelPipe_C";

    unsafe fn create(ptr: *mut Self::Wrapping) -> Self {
        PipeWrapper::new(ActorWrapper::new(ptr))
    }
}
impl<'a> Deref for PipeWrapper<'a> {
    type Target = ActorWrapper<'a>;

    fn deref(&self) -> &Self::Target {
        &self.base
    }
}
impl<'a> Pointer for PipeWrapper<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        Pointer::fmt(&self.base, f)
    }
}
impl<'a> PipeWrapper<'a> {
    pub fn new(pipe: ActorWrapper<'a>) -> PipeWrapper<'a> {
        assert_eq!(pipe.class().name(), "BP_TravelPipe_C");
        PipeWrapper { base: pipe }
    }
}

#[derive(Debug, Clone)]
pub struct SpringpadWrapper<'a> {
    base: ActorWrapper<'a>,
}
pub enum SpringpadWrapperType {}
impl UeObjectWrapperType for SpringpadWrapperType {
    type UeObjectWrapper<'a> = SpringpadWrapper<'a>;
}
unsafe impl<'a> UeObjectWrapper<'a> for SpringpadWrapper<'a> {
    type UeObjectWrapperType = SpringpadWrapperType;
    type Wrapping = AActor;
    const CLASS_NAME: &'static str = "BP_Jumppad_C";

    unsafe fn create(ptr: *mut Self::Wrapping) -> Self {
        SpringpadWrapper::new(ActorWrapper::new(ptr))
    }
}
impl<'a> Deref for SpringpadWrapper<'a> {
    type Target = ActorWrapper<'a>;

    fn deref(&self) -> &Self::Target {
        &self.base
    }
}
impl<'a> Pointer for SpringpadWrapper<'a> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        Pointer::fmt(&self.base, f)
    }
}
impl<'a> SpringpadWrapper<'a> {
    pub fn new(springpad: ActorWrapper<'a>) -> SpringpadWrapper<'a> {
        assert_eq!(springpad.class().name(), "BP_Jumppad_C");
        SpringpadWrapper { base: springpad }
    }
}

#[derive(Debug, Copy, Clone, rebo::ExternalType, PartialEq)]
pub enum ElementType {
    Platform,
    Cube,
    Button,
    Lift,
    Pipe,
    Springpad,
}

#[derive(Debug, Clone, Copy, rebo::ExternalType)]
pub struct ElementIndex {
    pub cluster_index: usize,
    pub element_type: ElementType,
    pub element_index: usize,
}


pub fn try_find_element_index(ptr: *mut UObject) -> Option<ElementIndex> {
    UeScope::with(|scope| {
        for (i, level) in LEVELS.lock().unwrap().iter().enumerate() {
            let found = level.platforms.iter().map(|p| (ElementType::Platform, scope.get(p).as_ptr() as usize)).enumerate()
                .chain(level.cubes.iter().map(|c| (ElementType::Cube, scope.get(c).as_ptr() as usize)).enumerate())
                .chain(level.buttons.iter().map(|c| (ElementType::Button, scope.get(c).as_ptr() as usize)).enumerate())
                .chain(level.lifts.iter().map(|c| (ElementType::Lift, scope.get(c).as_ptr() as usize)).enumerate())
                .chain(level.pipes.iter().map(|c| (ElementType::Pipe, scope.get(c).as_ptr() as usize)).enumerate())
                .chain(level.springpads.iter().map(|c| (ElementType::Springpad, scope.get(c).as_ptr() as usize)).enumerate())
                .find(|(_, (_, addr))| ptr as usize == *addr)
                .map(|(ei, (typ, _))| ElementIndex { cluster_index: i, element_type: typ, element_index: ei});
            if let Some(found) = found {
                return Some(found);
            }
        }
        None
    })
}

pub fn init() {
    UeScope::with(|scope| {
        let mut levels = LEVELS.lock().unwrap();
        let mut lifts = Vec::new();
        let mut pipes = Vec::new();
        let mut springpads = Vec::new();
        for item in scope.iter_global_object_array() {
            let object = item.object();
            let name = object.name();
            let class_name = object.class().name();
            // fn print_children(depth: usize, class: StructWrapper) {
            //     use crate::native::{PropertyWrapper, UProperty};
            //     use crate::native::reflection::{StructWrapper, ClassWrapper};
            //     for property in class.iter_properties() {
            //         let class_name = property.class().name();
            //         log!("{}{property}", "    ".repeat(depth));
            //         if class_name == "ObjectProperty" {
            //             let class = unsafe { ClassWrapper::new((*(property.as_uobjectproperty())).property_class) };
            //             log!("{}going into {}", "    ".repeat(depth), class.name());
            //             // print_children(depth+1, class);
            //         }
            //     }
            //     log!("{}done printing children", "    ".repeat(depth));
            // }
            // log!("{:?} {:?} ({object:p})", class_name, name);
            // print_children(1, object.class());

            if class_name == "BP_LevelRoot_C" && name != "Default__BP_LevelRoot_C" {
                let level: LevelWrapper = object.upcast();
                levels.push(Level {
                    level: scope.object_index(&level),
                    platforms: level.platforms().map(|p| scope.object_index(&p)).collect(),
                    cubes: level.cubes().map(|c| scope.object_index(&c)).collect(),
                    buttons: level.buttons().map(|b| scope.object_index(&b)).collect(),
                    lifts: vec![],
                    pipes: vec![],
                    springpads: vec![],
                })
            }
            if class_name == "BP_Lift_C" && name != "Default__BP_Lift_C" {
                let lift: LiftWrapper = object.upcast();
                lifts.push(lift);
            }
            if class_name == "BP_TravelPipe_C" && name != "Default__BP_TravelPipe_C" {
                let pipe: PipeWrapper = object.upcast();
                pipes.push(pipe);
            }
            if class_name == "BP_Jumppad_C" && name != "Default__BP_Jumppad_C" {
                let springpad: SpringpadWrapper = object.upcast();
                springpads.push(springpad);
            }
            if class_name == "BP_PowerCore_C" && name != "Default__BP_PowerCore_C" {
                let mesh = object.get_field("Mesh").unwrap::<ObjectWrapper>();
                let trigger = object.get_field("Trigger").unwrap::<ObjectWrapper>();
                let fun = mesh.class().find_function("SetCollisionResponseToChannel").unwrap();
                let params = fun.create_argument_struct();
                params.get_field("Channel").unwrap::<&Cell<u8>>().set(3); // Visibility channel
                params.get_field("NewResponse").unwrap::<&Cell<u8>>().set(2); // 'Block' response
                unsafe {
                    fun.call(trigger.as_ptr(), &params);
                }
            }
        }
        assert_eq!(levels.len(), 31);
        levels.sort_by_key(|level| scope.get(level.level).level_index());

        for lift in lifts {
            let level_index: usize = match lift.name().as_str() {
                "BP_Lift_C_1" => 5,
                "BP_Mover7" => 7,
                "BP_Mover6" => 8,
                name => unreachable!("Invalid lift: {name:?}"),
            };
            levels[level_index].lifts.push(scope.object_index(&lift));
        }

        for pipe in &pipes {
            let level_index: usize = match pipe.name().as_str() {
                "BP_TravelPipe_C_6" => 4,
                "BP_TravelPipe_C_5" => 4,
                "BP_TravelPipe_359" => 6,
                "BP_TravelPipe_C_3" => 9,
                "BP_TravelPipe_C_2" => 9,
                "BP_Pipe4" => 18,
                "BP_TravelPipe_C_1" => 18,
                "BP_TravelPipe2_855" => 23,
                "BP_TravelPipe_1816" => 25,
                name => unreachable!("Invalid pipe: {name:?}"),
            };
            levels[level_index].pipes.push(scope.object_index(pipe));
        }

        for pad in &springpads {
            let level_index: usize = match pad.name().as_str() {
                "BP_Jumppad_414" if pad.absolute_location().0 == 250. => 23,
                "BP_Jumppad2_530" => 24,
                "BP_Jumppad_414" if pad.absolute_location().0 == 2075. => 24,
                "BP_Jumppad_514" => 26,
                name => unreachable!("Invalid pipe: {name:?}"),
            };
            levels[level_index].springpads.push(scope.object_index(pad));
        }

        for lvl in &mut *levels {
            lvl.pipes.sort_by_key(|pipe| {
                let (x, y, z) = scope.get(pipe).absolute_location();
                (x as i32, y as i32, z as i32)
            });
        }
    })
}
