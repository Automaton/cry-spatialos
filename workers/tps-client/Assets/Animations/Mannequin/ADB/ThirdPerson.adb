<AnimDB FragDef="Animations/Mannequin/ADB/PlayerFragmentIds.xml" TagDef="Animations/Mannequin/ADB/PlayerTags.xml">
 <FragmentList>
  <Idle>
   <Fragment BlendOutDuration="0.2" Tags="Rotate">
    <AnimLayer>
     <Blend ExitTime="0" StartTime="0" Duration="0.2"/>
     <Animation name="stand_tac_stepRot180_rifle_3p_01" flags="Loop"/>
    </AnimLayer>
   </Fragment>
   <Fragment BlendOutDuration="0.2" Tags="Walk">
    <AnimLayer>
     <Blend ExitTime="0" StartTime="0" Duration="0.2"/>
     <Animation name="stand_tac_move_rifle_3p_01" flags="Loop"/>
    </AnimLayer>
    <AnimLayer>
     <Blend ExitTime="0" StartTime="0" Duration="0.2"/>
     <Animation name="stand_tac_weaponpose_rifle_3p_01" flags="Loop"/>
    </AnimLayer>
   </Fragment>
   <Fragment BlendOutDuration="0.2" Tags="Walk+Rotate">
    <AnimLayer>
     <Blend ExitTime="0" StartTime="0" Duration="0.2"/>
     <Animation name="stand_tac_move_rifle_3p_01" flags="Loop"/>
    </AnimLayer>
    <AnimLayer>
     <Blend ExitTime="0" StartTime="0" Duration="0.2"/>
     <Animation name="stand_tac_weaponpose_rifle_3p_01" flags="Loop"/>
    </AnimLayer>
   </Fragment>
   <Fragment BlendOutDuration="0.2" Tags="">
    <AnimLayer>
     <Blend ExitTime="0" StartTime="0" Duration="0.2"/>
     <Animation name="stand_tac_idle_rifle_3p_01" flags="Loop"/>
    </AnimLayer>
   </Fragment>
  </Idle>
  <aimPose>
   <Fragment BlendOutDuration="0.2">
    <ProcLayer>
     <Blend ExitTime="0" StartTime="0" Duration="0.2"/>
     <Procedural type="AimPose">
      <ProceduralParams>
       <Animation value="stand_tac_aimposes_idle_rifle_shoulder_3p_01"/>
       <BlendTime value="0.2"/>
       <AnimationLayer value="4"/>
      </ProceduralParams>
     </Procedural>
    </ProcLayer>
   </Fragment>
  </aimPose>
 </FragmentList>
</AnimDB>
