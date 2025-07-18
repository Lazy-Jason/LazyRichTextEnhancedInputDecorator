// // Copyright (C) 2024 Job Omondiale - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "LazyRichTextEnhancedInputDecorator.generated.h"

class ITextDecorator;
class UDataTable;
class UInputAction;
// Struct for data table rows mapping keys to images
USTRUCT()
struct LAZYGENERICRICHTEXTENHANCEDINPUT_API FKeyImageRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	FKey Key;

	UPROPERTY(EditAnywhere, Category = "Input")
	FSlateBrush Image;
};

/**
 * Custom rich text decorator for Enhanced Input key bindings
 */
UCLASS(Abstract)
class LAZYGENERICRICHTEXTENHANCEDINPUT_API ULazyRichTextEnhancedInputDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

	// Data table for keyboard and mouse key images
	UPROPERTY(EditAnywhere, Category = "Input Images")
	UDataTable* KeyboardMouseImages;

	// Map of gamepad hardware types to their respective image data tables
	UPROPERTY(EditAnywhere, Category = "Input Images")
	TMap<FName, UDataTable*> GamepadImages;

	// Weak reference to the owning rich text block
	TWeakObjectPtr<URichTextBlock> OwnerRichTextBlock;

	FText OriginalText = FText();

public:
	// New property to map action names to UInputAction objects
	UPROPERTY(EditAnywhere, Category = "Input")
	TMap<FName, UInputAction*> ActionMap;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Editor")
	FSlateBrush EditorBrush = FSlateBrush();
#endif

protected:
	virtual void BeginDestroy() override;
	
	// Handle input device changes to refresh the rich text block
	UFUNCTION()
	void HandleInputDeviceChanged(FPlatformUserId UserId, FInputDeviceId DeviceId);

	UFUNCTION()
	void HandleControlMappingsRebuilt();

public:
	ULazyRichTextEnhancedInputDecorator(const FObjectInitializer& ObjectInitializer);
	
	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

	// Find the appropriate image brush for a key based on the current device
	const FSlateBrush* FindImageBrush(const FKey& Key, const APlayerController* PlayerController);
};

/**
 * Inline decorator for rendering key images
 */
class LAZYGENERICRICHTEXTENHANCEDINPUT_API FLazyRichInlineInputImage : public FRichTextDecorator
{
public:
	FLazyRichInlineInputImage(URichTextBlock* InOwner, ULazyRichTextEnhancedInputDecorator* InDecorator)
		: FRichTextDecorator(InOwner), Decorator(InDecorator)
	{}

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;

protected:
	virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& TextStyle) const override;

private:
	// Get the current key bound to an action
	FKey GetKeyForAction(FName ActionName, APlayerController* PlayerController) const;

	ULazyRichTextEnhancedInputDecorator* Decorator;
};