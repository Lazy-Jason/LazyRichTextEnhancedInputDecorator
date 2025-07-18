// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#include "RichText/LazyRichTextEnhancedInputDecorator.h"
#include "EnhancedInputSubsystemInterface.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "Components/RichTextBlock.h"
#include "Fonts/FontMeasure.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "Misc/DefaultValueHelper.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "LazyRichText"

class SLazyRichInlineInputImage : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLazyRichInlineInputImage)
    {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FSlateBrush* Brush, const FTextBlockStyle& TextStyle, TOptional<int32> Width, TOptional<int32> Height, EStretch::Type Stretch)
    {
        check(Brush);

        const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        float IconHeight = FMath::Min(static_cast<float>(FontMeasure->GetMaxCharacterHeight(TextStyle.Font, 1.0f)), Brush->ImageSize.Y);

        if (Height.IsSet())
        {
            IconHeight = Height.GetValue();
        }

        float IconWidth = IconHeight;
        if (Width.IsSet())
        {
            IconWidth = Width.GetValue();
        }

        ChildSlot
        [
            SNew(SBox)
            .HeightOverride(IconHeight)
            .WidthOverride(IconWidth)
            [
                SNew(SScaleBox)
                .Stretch(Stretch)
                .StretchDirection(EStretchDirection::DownOnly)
                .VAlign(VAlign_Center)
                [
                    SNew(SImage)
                    .Image(Brush)
                ]
            ]
        ];
    }
};

ULazyRichTextEnhancedInputDecorator::ULazyRichTextEnhancedInputDecorator(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Register for device change notifications
    if (UInputDeviceSubsystem* Subsystem = UInputDeviceSubsystem::Get())
    {
        Subsystem->OnInputHardwareDeviceChanged.AddDynamic(this, &ULazyRichTextEnhancedInputDecorator::HandleInputDeviceChanged);
    }
}

TSharedPtr<ITextDecorator> ULazyRichTextEnhancedInputDecorator::CreateDecorator(URichTextBlock* InOwner)
{
    OwnerRichTextBlock = InOwner;
    OriginalText = InOwner->GetText();

    // Bind to control mapping rebuilds when the owner is set
    if (APlayerController* PC = InOwner->GetOwningPlayer())
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                Subsystem->ControlMappingsRebuiltDelegate.AddDynamic(this, &ULazyRichTextEnhancedInputDecorator::HandleControlMappingsRebuilt);
            }
        }
    }

    return MakeShareable(new FLazyRichInlineInputImage(InOwner, this));
}

void ULazyRichTextEnhancedInputDecorator::HandleInputDeviceChanged(FPlatformUserId UserId, FInputDeviceId DeviceId)
{
    /*if(UInputDeviceSubsystem* Subsystem = UInputDeviceSubsystem::Get())
    {
        FHardwareDeviceIdentifier NewDeviceIdentifier = Subsystem->GetInputDeviceHardwareIdentifier(DeviceId);
        if (GEngine)
        {
            GEngine -> AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Input device changed: %s"),
                *NewDeviceIdentifier.HardwareDeviceIdentifier.ToString()));
        }
    }*/

    // When input device changes, force a refresh of the rich text block
    if (OwnerRichTextBlock.IsValid())
    {
        // Get the Slate widget using StaticCastSharedRef instead of Cast<>
        TSharedPtr<SWidget> SlateWidget = OwnerRichTextBlock->GetCachedWidget();
        if (SlateWidget.IsValid())
        {
            TSharedPtr<SRichTextBlock> SlateRichText = StaticCastSharedPtr<SRichTextBlock>(SlateWidget);
            if (SlateRichText.IsValid())
            {
                // Clear and reset the text
                /*OwnerRichTextBlock->SetText(FText::GetEmpty());
                OwnerRichTextBlock->SetText(OriginalText);*/
                
                // Use the Refresh method on SRichTextBlock
                SlateRichText->Refresh();
            }
        }
    }
}

void ULazyRichTextEnhancedInputDecorator::HandleControlMappingsRebuilt()
{
    // When control mappings are rebuilt, also refresh the rich text block
    HandleInputDeviceChanged(FPlatformUserId(), FInputDeviceId());
}

void ULazyRichTextEnhancedInputDecorator::BeginDestroy()
{
    // Clean up delegates to prevent dangling pointers
    if (UInputDeviceSubsystem* Subsystem = UInputDeviceSubsystem::Get())
    {
        Subsystem->OnInputHardwareDeviceChanged.RemoveDynamic(this, &ULazyRichTextEnhancedInputDecorator::HandleInputDeviceChanged);
    }

    if (OwnerRichTextBlock.IsValid())
    {
        if (APlayerController* PC = OwnerRichTextBlock->GetOwningPlayer())
        {
            if (ULocalPlayer* LP = PC->GetLocalPlayer())
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                {
                    Subsystem->ControlMappingsRebuiltDelegate.RemoveDynamic(this, &ULazyRichTextEnhancedInputDecorator::HandleControlMappingsRebuilt);
                }
            }
        }
    }

    Super::BeginDestroy();
}

const FSlateBrush* ULazyRichTextEnhancedInputDecorator::FindImageBrush(const FKey& Key, const APlayerController* PlayerController)
{
    if (!Key.IsValid())
    {
        return nullptr;
    }

    UDataTable* DataTableToUse = nullptr;

    // Select the appropriate data table based on the current input device
    if (Key.IsGamepadKey())
    {
        if (UInputDeviceSubsystem* Subsystem = UInputDeviceSubsystem::Get())
        {
            FHardwareDeviceIdentifier DeviceId = Subsystem->GetMostRecentlyUsedHardwareDevice(PlayerController->GetPlatformUserId());
            if (DeviceId.IsValid())
            {
                if (UDataTable** FoundTable = GamepadImages.Find(DeviceId.HardwareDeviceIdentifier))
                {
                    DataTableToUse = *FoundTable;
                }
            }
        }
    }
    else
    {
        DataTableToUse = KeyboardMouseImages;
    }

    // Look up the image for the key in the data table
    if (DataTableToUse)
    {
        FString ContextString;
        if (FKeyImageRow* Row = DataTableToUse->FindRow<FKeyImageRow>(Key.GetFName(), ContextString))
        {
            return &Row->Image;
        }
    }

    return nullptr;
}

bool FLazyRichInlineInputImage::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const
{
    return RunParseResult.Name == TEXT("input") && RunParseResult.MetaData.Contains(TEXT("action"));
}

TSharedPtr<SWidget> FLazyRichInlineInputImage::CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& TextStyle) const
{
    if (Owner->IsDesignTime())
    {
        // Design time: Use a default brush for preview
        static FSlateBrush DefaultBrush;
        
        #if WITH_EDITORONLY_DATA
                // Use the editor brush if we're in the editor
                const FSlateBrush* BrushToUse = &Decorator->EditorBrush;
                TOptional<int32> NewWidth = Decorator->EditorBrush.ImageSize.X;
                TOptional<int32> NewHeight = Decorator->EditorBrush.ImageSize.Y;
        #else
                // Otherwise use the default brush
                const FSlateBrush* BrushToUse = &DefaultBrush;
                TOptional<int32> NewWidth;
                TOptional<int32> NewHeight;
        #endif
        
        return SNew(SLazyRichInlineInputImage, BrushToUse, TextStyle, NewWidth, NewHeight, EStretch::ScaleToFit);
    }
    
    // Extract action name
    FString ActionNameStr = RunInfo.MetaData[TEXT("action")];
    FName ActionName(*ActionNameStr);

    // Get PlayerController from the owning rich text block
    APlayerController* PlayerController = Owner->GetOwningPlayer();
    if (!PlayerController)
    {
        return nullptr;
    }

    // Get the current key for the action
    FKey Key = GetKeyForAction(ActionName, PlayerController);
    if (!Key.IsValid())
    {
        return nullptr;
    }

    // Find the image brush for the key
    const FSlateBrush* Brush = Decorator->FindImageBrush(Key, PlayerController);
    if (!Brush)
    {
        // Return null if no brush is found
        return nullptr;
    }

    // Parse width, height, and stretch from the meta data
    TOptional<int32> Width;
    if (const FString* WidthString = RunInfo.MetaData.Find(TEXT("width")))
    {
        int32 WidthTemp;
        if (FDefaultValueHelper::ParseInt(*WidthString, WidthTemp))
        {
            Width = WidthTemp;
        }
        else if (FCString::Stricmp(**WidthString, TEXT("desired")) == 0)
        {
            Width = Brush->ImageSize.X;
        }
    }

    TOptional<int32> Height;
    if (const FString* HeightString = RunInfo.MetaData.Find(TEXT("height")))
    {
        int32 HeightTemp;
        if (FDefaultValueHelper::ParseInt(*HeightString, HeightTemp))
        {
            Height = HeightTemp;
        }
        else if (FCString::Stricmp(**HeightString, TEXT("desired")) == 0)
        {
            Height = Brush->ImageSize.Y;
        }
    }

    EStretch::Type Stretch = EStretch::ScaleToFit;
    if (const FString* StretchString = RunInfo.MetaData.Find(TEXT("stretch")))
    {
        const UEnum* StretchEnum = StaticEnum<EStretch::Type>();
        int64 StretchValue = StretchEnum->GetValueByNameString(*StretchString);
        if (StretchValue != INDEX_NONE)
        {
            Stretch = static_cast<EStretch::Type>(StretchValue);
        }
    }

    // Create the inline image widget
    return SNew(SLazyRichInlineInputImage, Brush, TextStyle, Width, Height, Stretch);
}

FKey FLazyRichInlineInputImage::GetKeyForAction(FName ActionName, APlayerController* PlayerController) const
{
    UInputAction* Action = Decorator->ActionMap.FindRef(ActionName);
    if (!Action || !PlayerController)
    {
        return EKeys::Invalid;
    }

    if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(Action);
            if (Keys.IsEmpty())
            {
                return EKeys::Invalid;
            }

            // Determine the current input type
            bool bUsingGamepad = false;
            if (UInputDeviceSubsystem* DeviceSubsystem = UInputDeviceSubsystem::Get())
            {
                FHardwareDeviceIdentifier DeviceId = DeviceSubsystem->GetMostRecentlyUsedHardwareDevice(PlayerController->GetPlatformUserId());
                
                // Check if we have an explicitly detected keyboard/mouse
                bool bExplicitlyUsingKBM = DeviceId.IsValid() && 
                    (DeviceId.PrimaryDeviceType == EHardwareDevicePrimaryType::KeyboardAndMouse);
                
                // If not explicitly KBM, default to gamepad (covers Steam Deck and other cases)
                bUsingGamepad = !bExplicitlyUsingKBM;
                
                // Debug message to help identify the device
                /*if (GEngine && DeviceId.IsValid())
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                        FString::Printf(TEXT("Device: %s, Type: %d, Using Gamepad: %s"), 
                        *DeviceId.HardwareDeviceIdentifier.ToString(),
                        static_cast<int32>(DeviceId.PrimaryDeviceType),
                        bUsingGamepad ? TEXT("True") : TEXT("False")));
                }*/
            }

            // First pass: Try to find a key matching the current input device type
            for (const FKey& Key : Keys)
            {
                if (!Key.IsValid())
                {
                    continue;
                }
                if (bUsingGamepad && Key.IsGamepadKey())
                {
                    return Key; // Return gamepad key if using a gamepad
                }
                else if (!bUsingGamepad && !Key.IsGamepadKey() && !Key.IsTouch())
                {
                    return Key; // Return keyboard/mouse key if not using a gamepad
                }
            }
            
            // Fallback: Return the first valid key
            for (const FKey& Key : Keys)
            {
                if (Key.IsValid())
                {
                    return Key;
                }
            }
        }
    }

    return EKeys::Invalid;
}

#undef LOCTEXT_NAMESPACE